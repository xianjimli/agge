#pragma once

#include "services.h"

#include <agge.text/text_engine.h>

class font_loader : public agge::text_engine_base::loader, agge::noncopyable
{
public:
	font_loader(services &s);

	virtual agge::font::accessor_ptr load(const wchar_t *typeface, int height, bool bold, bool italic,
		agge::font::key::grid_fit grid_fit);

private:
	services &_services;
};

class native_font_loader : public agge::text_engine_base::loader
{
	virtual agge::font::accessor_ptr load(const wchar_t *typeface, int height, bool bold, bool italic,
		agge::font::key::grid_fit grid_fit);
};
