#pragma once
#ifndef I_LCD_HAL_H
#define I_LCD_HAL_H

#include "Common.h"

class I_LCD_HAL
{
public:
	enum class Orientations
	{
		Upright = 0,
		UpsideDown,
		ToRight,
		ToLeft
	};

public:
	virtual void Update(void) = 0;

	virtual void SetTargetFrameRate(uint8 Value) = 0;
	virtual uint8 GetTargetFrameRate(void) const = 0;

	virtual void Clear(Color Color) = 0;

	virtual void DrawPixel(Point Position, Color Color) = 0;

	virtual const Point &GetDimension(void) const = 0;
};

#endif