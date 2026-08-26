#pragma once
#ifndef FONT_H
#define FONT_H

#include "Point.h"

struct Font
{
public:
	typedef uint64 DataType;

public:
	uint8 MaxWidth;
	uint8 Height;
	const DataType* const Data;
	float Scale;
	uint8 BitsPerPixel;
	bool HasGlyphData;
	cstr Glyphs;

public:
	Point GetScaledSize(void) const
	{
		return Point(MaxWidth, Height) * Scale;
	}

public:
	static constexpr Font CreateScaled(const Font& ReferenceFont, uint8 TargetHeight)
	{
		Font font = ReferenceFont;
		font.Scale = (float)TargetHeight / ReferenceFont.Height;
		return font;
	}
};

#endif