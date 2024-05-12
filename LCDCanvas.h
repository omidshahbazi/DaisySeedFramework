#pragma once

#include "I_LCD_HAL.h"

class LCDCanvas
{
public:
	LCDCanvas(I_LCD_HAL *HAL)
		: m_HAL(HAL)
	{
	}

	void Clear(I_LCD_HAL::Color Color)
	{
		m_HAL->Clear(Color);
	}

	void DrawPixel(uint16 X, uint16 Y, I_LCD_HAL::Color Color)
	{
		m_HAL->DrawPixel(X, Y, Color);
	}

	void DrawArea(uint16 X, uint16 Y, uint16 Width, uint16 Height, I_LCD_HAL::Color Color)
	{
		for (uint32 yy = 0; yy < Height; ++yy)
			for (uint32 xx = 0; xx < Width; ++xx)
				DrawPixel(X + xx, Y + yy, Color);
	}

private:
	I_LCD_HAL *m_HAL;
};