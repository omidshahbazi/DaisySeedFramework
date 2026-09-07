#pragma once
#ifndef FONT_H
#define FONT_H

#include "Point.h"

struct Font
{
public:
	typedef uint64_t DataType;

public:
	uint8_t MaxWidth;
	uint8_t Height;
	const DataType* const Data;
	float Scale;
	uint8_t BitsPerPixel;
	bool HasGlyphData;
	cstr Glyphs;

public:
	Point GetScaledSize(void) const
	{
		return Point(MaxWidth, Height) * Scale;
	}

public:
	static constexpr Font CreateScaled(const Font& ReferenceFont, uint8_t TargetHeight)
	{
		Font font = ReferenceFont;
		font.Scale = (float)TargetHeight / ReferenceFont.Height;
		return font;
	}
};

#endif