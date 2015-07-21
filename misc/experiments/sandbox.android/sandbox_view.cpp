/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <aggx/rasterizer.h>
#include <aggx/blenders.h>

#include <aggx/agg_conv_stroke.h>

#include <agge/scanline.h>
#include <agg/include/agg_rasterizer_sl_clip.h>

#include <jni.h>
#include <android/bitmap.h>
#include <memory>

using namespace aggx;
using namespace std;

namespace
{
	agge::pixel32 make_pixel(rgba8 color)
	{
		agge::pixel32 p = { color.r, color.g, color.b, 0 };
		return p;
	}

	template <typename BlenderT>
	class blenderx : public BlenderT
	{
	public:
		typedef typename BlenderT::cover_type cover_type;

	public:
		blenderx(rgba8 color)
			: BlenderT(make_pixel(color), color.a)
		{	}
	};

	class bitmap_proxy
	{
	public:
		typedef agge::pixel32 pixel;

	public:
		bitmap_proxy(JNIEnv *env, jobject bitmap)
			: _env(env), _bitmap(bitmap), _memory(NULL)
		{
			AndroidBitmapInfo info;

			if (AndroidBitmap_getInfo(env, bitmap, &info) < 0 || info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
				throw 0;
			_width = info.width;
			_height = info.height;
			_stride = info.stride / sizeof(pixel);
			if (AndroidBitmap_lockPixels(env, bitmap, &_memory) < 0 || !_memory)
				throw 0;
		}

		~bitmap_proxy()
		{
			AndroidBitmap_unlockPixels(_env, _bitmap);
		}

		pixel *row_ptr(unsigned y)
		{	return reinterpret_cast<pixel *>(_memory) + y * _stride;	}

		unsigned width() const
		{	return _width;	}

		unsigned height() const
		{	return _height;	}

	private:
		bitmap_proxy(const bitmap_proxy &other);
		const bitmap_proxy &operator =(const bitmap_proxy &other);

	private:
		JNIEnv * const _env;
		const jobject _bitmap;
		unsigned _width, _height, _stride;
		void *_memory;
	};


	class spiral
	{
	public:
		spiral(real x, real y, real r1, real r2, real step, real start_angle) :
		  m_x(x), 
			  m_y(y), 
			  m_r1(r1), 
			  m_r2(r2), 
			  m_step(step), 
			  m_start_angle(start_angle),
			  m_angle(start_angle),
			  m_da(deg2rad(1.0)),
			  m_dr(m_step / 45.0f)
		  {
		  }

		  void rewind(unsigned) 
		  { 
			  m_angle = m_start_angle; 
			  m_curr_r = m_r1; 
			  m_start = true; 
		  }

		  unsigned vertex(real* x, real* y)
		  {
			  if(m_curr_r > m_r2) return path_cmd_stop;

			  *x = m_x + aggx::cos(m_angle) * m_curr_r;
			  *y = m_y + aggx::sin(m_angle) * m_curr_r;
			  m_curr_r += m_dr;
			  m_angle += m_da;
			  if(m_start) 
			  {
				  m_start = false;
				  return path_cmd_move_to;
			  }
			  return path_cmd_line_to;
		  }

	private:
		real m_x;
		real m_y;
		real m_r1;
		real m_r2;
		real m_step;
		real m_start_angle;

		real m_angle;
		real m_curr_r;
		real m_da;
		real m_dr;
		bool   m_start;
	};

	typedef blenderx<blender_solid_color> blender;
	typedef agge::rendition_adapter<bitmap_proxy, blender> renderer;
	typedef rasterizer_scanline_aa<agg::rasterizer_sl_no_clip/*agg::rasterizer_sl_clip_int*/> rasterizer_scanline;
	typedef agge::scanline_adapter<renderer> scanline;

	struct AGG
	{
	public:
		rasterizer_scanline rasterizer;
	};
}



extern "C" JNIEXPORT void JNICALL Java_impression_sandbox_SandboxView_constructAGG(JNIEnv *env, jobject obj)
try
{
	AGG *ptr = new AGG;
	jfieldID fieldidAGG = env->GetFieldID(env->GetObjectClass(obj), "aggObject", "J");
	env->SetLongField(obj, fieldidAGG, reinterpret_cast<jlong>(ptr));
}
catch (...)
{
}

extern "C" JNIEXPORT void JNICALL Java_impression_sandbox_SandboxView_destroyAGG(JNIEnv *env, jobject obj)
{
	jfieldID fieldidAGG = env->GetFieldID(env->GetObjectClass(obj), "aggObject", "J");
	delete reinterpret_cast<AGG*>(env->GetLongField(obj, fieldidAGG));
	env->SetLongField(obj, fieldidAGG, 0);
}

extern "C" JNIEXPORT void JNICALL Java_impression_sandbox_SandboxView_render(JNIEnv *env, jobject obj, jobject bitmap)
try
{
	jfieldID fieldidAGG = env->GetFieldID(env->GetObjectClass(obj), "aggObject", "J");
	AGG* agg = reinterpret_cast<AGG*>(env->GetLongField(obj, fieldidAGG));

	bitmap_proxy bm(env, bitmap);
	rasterizer_scanline ras;

	spiral s4(bm.width() / 2, bm.height() / 2, 5, (min)(bm.width(), bm.height()) / 2 - 10, 1, 0);
	conv_stroke<spiral> stroke(s4);

	stroke.width(3);
	stroke.line_cap(round_cap);
	stroke.line_join(bevel_join);
	agg->rasterizer.add_path(stroke);
	agg->rasterizer.prepare();

	renderer r(bm, blender(rgba8(0, 154, 255, 255)));

	agg->rasterizer.render<scanline>(r);
}
catch (...)
{
}
