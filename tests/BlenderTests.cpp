#include <agge/blenders_simd.h>

#include "helpers.h"

#include <utee/ut/assert.h>
#include <utee/ut/test.h>

using namespace std;

namespace agge
{
	namespace tests
	{
		namespace
		{
			const pixel32 black = make_pixel(0x00, 0x00, 0x00, 0x00);
			const pixel32 white = make_pixel(0xFF, 0xFF, 0xFF, 0xFF);
		}

		begin_test_suite( SIMDBlendersTests )
			
			typedef simd::blender_solid_color blender;

			test( BlendingFullyOpaquePixelsOverwritesPreviousData )
			{
				// INIT
				const blender b001181FF(make_pixel(0x00, 0x11, 0x81, 0xFF), 0xFF);
				const blender b1181FF00(make_pixel(0x11, 0x81, 0xFF, 0x00), 0xFF);
				const blender b81FF0011(make_pixel(0x81, 0xFF, 0x00, 0x11), 0xFF);
				const blender bFF001181(make_pixel(0xFF, 0x00, 0x11, 0x81), 0xFF);
				const blender::cover_type guarded_cover[] = { 0x00, 0x00, 0x00, 0xFF, 0x00, 0x00, 0x00, };
				const blender::cover_type *cover = &guarded_cover[3];
				aligned_array<pixel32, 11> buffer;
				
				buffer.fill(black);

				// ACT
				b001181FF(&buffer[0], 0, 0, 1, cover);
				b1181FF00(&buffer[1], 0, 0, 1, cover);
				b81FF0011(&buffer[2], 0, 0, 1, cover);
				bFF001181(&buffer[3], 0, 0, 1, cover);
				bFF001181(&buffer[4], 0, 0, 1, cover);
				b81FF0011(&buffer[5], 0, 0, 1, cover);
				b1181FF00(&buffer[6], 0, 0, 1, cover);
				b001181FF(&buffer[7], 0, 0, 1, cover);

				// ASSERT
				const pixel32 reference1[] = {
					{ 0x00, 0x11, 0x81, 0xFF }, { 0x11, 0x81, 0xFF, 0x00 }, { 0x81, 0xFF, 0x00, 0x11 }, { 0xFF, 0x00, 0x11, 0x81 },
					{ 0xFF, 0x00, 0x11, 0x81 }, { 0x81, 0xFF, 0x00, 0x11 }, { 0x11, 0x81, 0xFF, 0x00 }, { 0x00, 0x11, 0x81, 0xFF },
					{ 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
				};

				assert_equal(reference1, buffer);

				// INIT
				buffer.fill(white);

				// ACT
				b001181FF(&buffer[0], 0, 0, 1, cover);
				b1181FF00(&buffer[1], 0, 0, 1, cover);
				b001181FF(&buffer[2], 0, 0, 1, cover);
				bFF001181(&buffer[3], 0, 0, 1, cover);
				bFF001181(&buffer[4], 0, 0, 1, cover);
				b81FF0011(&buffer[5], 0, 0, 1, cover);
				b1181FF00(&buffer[6], 0, 0, 1, cover);
				b81FF0011(&buffer[7], 0, 0, 1, cover);

				// ASSERT (yes, the black-on-white is 1-biased for the whole range, except '0'-cover)
				const pixel32 reference2[] = {
					{ 0x01, 0x12, 0x82, 0xFF }, { 0x12, 0x82, 0xFF, 0x01 }, { 0x01, 0x12, 0x82, 0xFF }, { 0xFF, 0x01, 0x12, 0x82 },
					{ 0xFF, 0x01, 0x12, 0x82 }, { 0x82, 0xFF, 0x01, 0x12 }, { 0x12, 0x82, 0xFF, 0x01 }, { 0x82, 0xFF, 0x01, 0x12 },
					{ 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF },
				};

				assert_equal(reference2, buffer);
			}


			test( BlendingErrorCheck )
			{
				// INIT
				const blender whiteness(white, 0xFE); // Pure 'white'.
				const blender blackness(black, 0xFE); // Pure 'black'.
				blender::cover_type covers[] = { 0x00, 0x00, 0x00, 0x00, };
				aligned_array<pixel32, 4> buffer;
				vector<int> diff;// = 0;

				// ACT
				for (int i = 0x00; i <= 0xFF; ++i)
				{
					buffer[0] = black;
					*covers = (uint8_t)i;
					whiteness(&buffer[0], 0, 0, 1, covers);
					diff.push_back(buffer[0].c0 - i);
				}

				diff.clear();

				for (int i = 0x00; i <= 0xFF; ++i)
				{
					buffer[0] = white;
					*covers = (uint8_t)i;
					blackness(&buffer[0], 0, 0, 1, covers);
					diff.push_back(int(buffer[0].c0) - int(0xFF - i));
				}

				// ASSERT (manual)
			}

			test( BlendingPurePixelsOfDifferentCoversCorrespondsOneToOne )
			{
				// INIT
				const blender whiteness(white, 0xFF); // Pure 'white'.
				const blender blackness(black, 0xFF); // Pure 'black'.
				const blender::cover_type covers1[] = { 0xFF, 0xA1, 0x13, 0x6C, };
				const blender::cover_type covers2[] = { 0x1F, 0x3A, 0x09, 0xF1, };
				aligned_array<pixel32, 4> buffer;

				buffer.fill(black);

				// ACT (on black)
				whiteness(&buffer[0], 0, 0, 4, covers1);

				// ASSERT
				const pixel32 reference1[] = {
					{ 0xFF, 0xFF, 0xFF, 0xFF }, { 0xA1, 0xA1, 0xA1, 0xA1 }, { 0x13, 0x13, 0x13, 0x13 }, { 0x6C, 0x6C, 0x6C, 0x6C },
				};

				assert_equal(reference1, buffer);

				// INIT
				buffer.fill(black);

				// ACT (on black)
				whiteness(&buffer[0], 0, 0, 4, covers2);

				// ASSERT
				const pixel32 reference2[] = {
					{ 0x1F, 0x1F, 0x1F, 0x1F }, { 0x3A, 0x3A, 0x3A, 0x3A }, { 0x09, 0x09, 0x09, 0x09 }, { 0xF1, 0xF1, 0xF1, 0xF1 },
				};

				assert_equal(reference2, buffer);

				// INIT
				buffer.fill(black);

				// ACT (on black)
				blackness(&buffer[0], 0, 0, 4, covers1);

				// ASSERT
				const pixel32 reference3[] = {
					{ 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
				};

				assert_equal(reference3, buffer);

				// ACT (on black)
				blackness(&buffer[0], 0, 0, 4, covers2);

				// ASSERT
				assert_equal(reference3, buffer);

				// INIT
				buffer.fill(white);

				// ACT (on white)
				blackness(&buffer[0], 0, 0, 4, covers1);

				// ASSERT
				const pixel32 reference4[] = {
					{ 0x01, 0x01, 0x01, 0x01 }, { 0x5F, 0x5F, 0x5F, 0x5F }, { 0xED, 0xED, 0xED, 0xED }, { 0x94, 0x94, 0x94, 0x94 },
				};

				assert_equal(reference4, buffer);

				// INIT
				buffer.fill(white);

				// ACT (on white)
				blackness(&buffer[0], 0, 0, 4, covers2);

				// ASSERT
				const pixel32 reference5[] = {
					{ 0xE1, 0xE1, 0xE1, 0xE1 }, { 0xC6, 0xC6, 0xC6, 0xC6 }, { 0xF7, 0xF7, 0xF7, 0xF7 }, { 0x0F, 0x0F, 0x0F, 0x0F },
				};

				assert_equal(reference5, buffer);

				// INIT
				buffer.fill(white);

				// ACT (on white)
				whiteness(&buffer[0], 0, 0, 4, covers1);

				// ASSERT
				const pixel32 reference6[] = {
					{ 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF },
				};

				assert_equal(reference6, buffer);

				// ACT (on white)
				whiteness(&buffer[0], 0, 0, 4, covers2);

				// ASSERT
				assert_equal(reference6, buffer);
			}


			test( BlendingSemiTransparentPixelsOfOpaqueCoverMakesOneToOneCorrespondence )
			{
				// INIT
				const blender c1(white, 0xF0);
				const blender c2(black, 0x2F);
				const blender::cover_type covers[] = { 0xFF, 0xFF, 0xFF, 0xFF, };
				aligned_array<pixel32, 4> buffer;

				buffer.fill(black);

				// ACT (on black)
				c1(&buffer[0], 0, 0, 4, covers);

				// ASSERT
				const pixel32 reference1[] = {
					{ 0xF0, 0xF0, 0xF0, 0xF0 }, { 0xF0, 0xF0, 0xF0, 0xF0 }, { 0xF0, 0xF0, 0xF0, 0xF0 }, { 0xF0, 0xF0, 0xF0, 0xF0 },
				};

				assert_equal(reference1, buffer);

				// INIT
				buffer.fill(black);

				// ACT (on black)
				c2(&buffer[0], 0, 0, 4, covers);

				// ASSERT
				const pixel32 reference2[] = {
					{ 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
				};

				assert_equal(reference2, buffer);

				// INIT
				buffer.fill(white);

				// ACT (on white)
				c1(&buffer[0], 0, 0, 4, covers);

				// ASSERT
				const pixel32 reference3[] = {
					{ 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF },
				};

				assert_equal(reference3, buffer);

				// INIT
				buffer.fill(white);

				// ACT (on white)
				c2(&buffer[0], 0, 0, 4, covers);

				// ASSERT
				const pixel32 reference4[] = {
					{ 0xD1, 0xD1, 0xD1, 0xD1 }, { 0xD1, 0xD1, 0xD1, 0xD1 }, { 0xD1, 0xD1, 0xD1, 0xD1 }, { 0xD1, 0xD1, 0xD1, 0xD1 },
				};

				assert_equal(reference4, buffer);
			}


			test( BlendingColorAlphaAndCoverAreMultiplied )
			{
				// INIT
				const blender b(make_pixel(0x1F, 0x32, 0x49, 0x9B), 0xF0);
				const blender::cover_type covers[] = { 0x11, 0x51, 0xB7, 0xE9, };
				aligned_array<pixel32, 4> buffer;

				buffer.fill(black);

				// ACT (on black)
				b(&buffer[0], 0, 0, 4, covers);

				// ASSERT
				const pixel32 reference1[] = {
					{ 0x02, 0x04, 0x05, 0x0a }, { 0x0a, 0x0f, 0x16, 0x2f }, { 0x15, 0x22, 0x32, 0x69 }, { 0x1b, 0x2b, 0x3f, 0x86 },
				};

				assert_equal(reference1, buffer);

				// INIT
				buffer.fill(make_pixel(0x1F, 0x5F, 0x80, 0x7F));

				// ACT
				b(&buffer[0], 0, 0, 4, covers);

				// ASSERT
				const pixel32 reference2[] = {
					{ 0x1F, 0x5D, 0x7D, 0x81 }, { 0x1F, 0x52, 0x70, 0x88 }, { 0x1F, 0x41, 0x5B, 0x92 }, { 0x1F, 0x39, 0x51, 0x98 },
				};

				assert_equal(reference2, buffer);
			}


			ignore( PixelsAlignmentIsObeyedOnBlending )
			{
				// INIT
				const blender b(white, 0xFF);
				const blender::cover_type covers_guarded[] = { 0x01, 0x01, 0x01, 0xFF, 0x01, 0x01, 0x01 };
				const blender::cover_type *covers = &covers_guarded[3];
				aligned_array<pixel32, 12> buffer;

				buffer.fill(black);

				// ACT
				b(&buffer[4], 0, 0, 1, covers);

				// ASSERT
				const pixel32 reference1[] = {
					{ 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
					{ 0xFF, 0xFF, 0xFF, 0xFF }, { 0x01, 0x01, 0x01, 0x01 }, { 0x01, 0x01, 0x01, 0x01 }, { 0x01, 0x01, 0x01, 0x01 },
					{ 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
				};

				assert_equal(reference1, buffer);

				// INIT
				buffer.fill(black);

				// ACT
				b(&buffer[5], 0, 0, 1, covers);

				// ASSERT
				const pixel32 reference2[] = {
					{ 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
					{ 0x01, 0x01, 0x01, 0x01 }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0x01, 0x01, 0x01, 0x01 }, { 0x01, 0x01, 0x01, 0x01 },
					{ 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
				};

				assert_equal(reference2, buffer);

				// INIT
				buffer.fill(black);

				// ACT
				b(&buffer[6], 0, 0, 1, covers);

				// ASSERT
				const pixel32 reference3[] = {
					{ 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
					{ 0x01, 0x01, 0x01, 0x01 }, { 0x01, 0x01, 0x01, 0x01 }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0x01, 0x01, 0x01, 0x01 },
					{ 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
				};

				assert_equal(reference3, buffer);

				// INIT
				buffer.fill(black);

				// ACT
				b(&buffer[7], 0, 0, 1, covers);

				// ASSERT
				const pixel32 reference4[] = {
					{ 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
					{ 0x01, 0x01, 0x01, 0x01 }, { 0x01, 0x01, 0x01, 0x01 }, { 0x01, 0x01, 0x01, 0x01 }, { 0xFF, 0xFF, 0xFF, 0xFF },
					{ 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
				};

				assert_equal(reference4, buffer);
			}


			ignore( NeighboringQuadsAreUpdated )
			{
				// INIT
				const blender b(white, 0xFF);
				const blender::cover_type covers_guarded[] = { 0x01, 0x01, 0x01, 0xFF, 0xFF, 0x01, 0x01, 0x01 };
				const blender::cover_type *covers = &covers_guarded[3];
				aligned_array<pixel32, 8> buffer;

				buffer.fill(black);

				// ACT
				b(&buffer[0], 0, 0, 2, covers);

				// ASSERT
				const pixel32 reference1[] = {
					{ 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0x01, 0x01, 0x01, 0x01 }, { 0x01, 0x01, 0x01, 0x01 },
					{ 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
				};

				assert_equal(reference1, buffer);

				// INIT
				buffer.fill(black);

				// ACT
				b(&buffer[3], 0, 0, 2, covers);

				// ASSERT
				const pixel32 reference2[] = {
					{ 0x01, 0x01, 0x01, 0x01 }, { 0x01, 0x01, 0x01, 0x01 }, { 0x01, 0x01, 0x01, 0x01 }, { 0xFF, 0xFF, 0xFF, 0xFF },
					{ 0xFF, 0xFF, 0xFF, 0xFF }, { 0x01, 0x01, 0x01, 0x01 }, { 0x01, 0x01, 0x01, 0x01 }, { 0x01, 0x01, 0x01, 0x01 },
				};

				assert_equal(reference2, buffer);
			}


			test( ZeroCoversCountChangesNothing )
			{
				// INIT
				const blender b(white, 0xFF);
				const blender::cover_type covers[] = { 0xFF, 0x01, 0x01, 0x01 };
				aligned_array<pixel32, 4> buffer;

				buffer.fill(black);

				// ACT
				b(&buffer[0], 0, 0, 0, covers);

				// ASSERT
				const pixel32 reference[] = {
					{ 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
				};

				assert_equal(reference, buffer);
			}


			test( TheNecessaryAmountOfQuadsAreUpdated )
			{
				// INIT
				const blender b(white, 0xFF);
				const blender::cover_type covers5[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x01, 0x01 };
				const blender::cover_type covers9[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01, 0x01, 0x01 };
				const blender::cover_type covers15[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01 };
				aligned_array<pixel32, 16> buffer;

				buffer.fill(black);

				// ACT
				b(&buffer[0], 0, 0, 5, covers5);

				// ASSERT
				const pixel32 reference1[] = {
					{ 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF },
					{ 0xFF, 0xFF, 0xFF, 0xFF }, { 0x01, 0x01, 0x01, 0x01 }, { 0x01, 0x01, 0x01, 0x01 }, { 0x01, 0x01, 0x01, 0x01 },
					{ 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
					{ 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
				};

				assert_equal(reference1, buffer);

				// INIT
				buffer.fill(black);

				// ACT
				b(&buffer[0], 0, 0, 9, covers9);

				// ASSERT
				const pixel32 reference2[] = {
					{ 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF },
					{ 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF },
					{ 0xFF, 0xFF, 0xFF, 0xFF }, { 0x01, 0x01, 0x01, 0x01 }, { 0x01, 0x01, 0x01, 0x01 }, { 0x01, 0x01, 0x01, 0x01 },
					{ 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
				};

				assert_equal(reference2, buffer);

				// INIT
				buffer.fill(black);

				// ACT
				b(&buffer[0], 0, 0, 15, covers15);

				// ASSERT
				const pixel32 reference3[] = {
					{ 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF },
					{ 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF },
					{ 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF },
					{ 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0x01, 0x01, 0x01, 0x01 },
				};

				assert_equal(reference3, buffer);
			}


			test( PlainFillOperatorIgnoresAlpha )
			{
				// INIT
				const blender b(make_pixel(0x01, 0x20, 0x32, 0x99), 0x51);
				aligned_array<pixel32, 4> buffer;

				buffer.fill(black);

				// ACT
				b(&buffer[0], 0, 0, 3);

				// ASSERT
				const pixel32 reference[] = {
					{ 0x01, 0x20, 0x32, 0x99 }, { 0x01, 0x20, 0x32, 0x99 }, { 0x01, 0x20, 0x32, 0x99 }, { 0x00, 0x00, 0x00, 0x00 },
				};

				assert_equal(reference, buffer);
			}


			test( PlainFillOperatorCopiesTheNecessaryAmountQuadsAtProperPosition )
			{
				// INIT
				const blender b(white, 0xFF);
				aligned_array<pixel32, 12> buffer;

				buffer.fill(black);

				// ACT
				b(&buffer[1], 0, 0, 2);

				// ASSERT
				const pixel32 reference1[] = {
					{ 0x00, 0x00, 0x00, 0x00 }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0x00, 0x00, 0x00, 0x00 },
					{ 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
					{ 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
				};

				assert_equal(reference1, buffer);

				// INIT
				buffer.fill(black);

				// ACT
				b(&buffer[5], 0, 0, 7);

				// ASSERT
				const pixel32 reference2[] = {
					{ 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 }, { 0x00, 0x00, 0x00, 0x00 },
					{ 0x00, 0x00, 0x00, 0x00 }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF },
					{ 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF }, 
				};

				assert_equal(reference2, buffer);
			}

		end_test_suite
	}
}