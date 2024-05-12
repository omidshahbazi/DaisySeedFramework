#pragma once

#include "I_LCD_HAL.h"
#include <daisy_seed.h>
#include <string>

class LCDCanvas
{
public:
	LCDCanvas(I_LCD_HAL *HAL)
		: m_HAL(HAL),
		  m_CharacterSpacing(0),
		  m_LineSpacing(0)
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

	void DrawLine(uint16 X0, uint16 Y0, uint16 X1, uint16 Y1, I_LCD_HAL::Color Color)
	{
		if (X0 == X1)
		{
			DrawVerticalLine(X0, Y0, Y1 - Y0 + 1, Color);

			return;
		}

		if (Y0 == Y1)
		{
			DrawHorizontalLine(X0, Y0, X1 - X0 + 1, Color);

			return;
		}

		int32 deltaX = Math::Absolute((int16)X1 - (int16)X0);
		int32 deltaY = Math::Absolute((int16)Y1 - (int16)Y0);
		int32 signX = Math::Sign(deltaX);
		int32 signY = Math::Sign(deltaY);
		int32 error = deltaX - deltaY;

		DrawPixel(X1, Y1, Color);

		while ((X0 != X1) || (Y0 != Y1))
		{
			DrawPixel(X0, Y0, Color);

			int32 error2 = error * 2;

			if (error2 > -deltaY)
			{
				error -= deltaY;
				X0 += signX;
			}

			if (error2 < deltaX)
			{
				error += deltaX;
				Y0 += signY;
			}
		}
	}

	void DrawRectangle(uint16 X, uint16 Y, uint16 Width, uint16 Height, I_LCD_HAL::Color Color)
	{
		int32 x2 = X + Width;
		int32 y2 = Y + Height;

		DrawLine(X, Y, X, y2, Color);
		DrawLine(X, Y, x2, Y, Color);
		DrawLine(X, y2, x2, y2, Color);
		DrawLine(x2, Y, x2, y2, Color);
	}

	void DrawFilledRectangle(uint16 X, uint16 Y, uint16 Width, uint16 Height, I_LCD_HAL::Color Color)
	{
		for (uint32 j = 0; j < Height; ++j)
			for (uint32 i = 0; i < Width; ++i)
				DrawPixel(X + i, Y + j, Color);
	}

	void DrawTriangle(uint16 X0, uint16 Y0, uint16 X1, uint16 Y1, uint16 X2, uint16 Y2, I_LCD_HAL::Color Color)
	{
		DrawLine(X0, Y0, X1, Y1, Color);
		DrawLine(X1, Y1, X2, Y2, Color);
		DrawLine(X2, Y2, X0, Y0, Color);
	}

	void DrawFilledTriangle(uint16 X0, uint16 Y0, uint16 X1, uint16 Y1, uint16 X2, uint16 Y2, I_LCD_HAL::Color Color)
	{
		int16 a, b, y, last;

		// Sort coordinates by Y order (y2 >= y1 >= y0)
		if (Y0 > Y1)
		{
			std::swap(Y0, Y1);
			std::swap(X0, X1);
		}
		if (Y1 > Y2)
		{
			std::swap(Y2, Y1);
			std::swap(X2, X1);
		}
		if (Y0 > Y1)
		{
			std::swap(Y0, Y1);
			std::swap(X0, X1);
		}

		if (Y0 == Y2)
		{
			// Handle awkward all-on-same-line case as its own thing
			a = b = X0;

			if (X1 < a)
				a = X1;
			else if (X1 > b)
				b = X1;

			if (X2 < a)
				a = X2;
			else if (X2 > b)
				b = X2;

			DrawHorizontalLine(a, Y0, b - a + 1, Color);

			return;
		}

		int16 dx01 = X1 - X0, dy01 = Y1 - Y0, dx02 = X2 - X0, dy02 = Y2 - Y0,
			  dx12 = X2 - X1, dy12 = Y2 - Y1;
		int32 sa = 0, sb = 0;

		// For upper part of triangle, find scanline crossings for segments
		// 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
		// is included here (and second loop will be skipped, avoiding a /0
		// error there), otherwise scanline y1 is skipped here and handled
		// in the second loop...which also avoids a /0 error here if y0=y1
		// (flat-topped triangle).
		if (Y1 == Y2)
		{
			last = Y1; // Include y1 scanline
		}
		else
		{
			last = Y1 - 1; // Skip it
		}

		for (y = Y0; y <= last; ++y)
		{
			a = X0 + sa / dy01;
			b = X0 + sb / dy02;
			sa += dx01;
			sb += dx02;
			/* longhand:
			a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
			b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
			*/
			if (a > b)
			{
				std::swap(a, b);
			}

			DrawHorizontalLine(a, y, b - a + 1, Color);
		}

		// For lower part of triangle, find scanline crossings for segments
		// 0-2 and 1-2.  This loop is skipped if y1=y2.
		sa = (int32)dx12 * (y - Y1);
		sb = (int32)dx02 * (y - Y0);
		for (; y <= Y2; ++y)
		{
			a = X1 + sa / dy12;
			b = X0 + sb / dy02;
			sa += dx12;
			sb += dx02;
			/* longhand:
				a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
				b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
			*/
			if (a > b)
			{
				std::swap(a, b);
			}

			DrawHorizontalLine(a, y, b - a + 1, Color);
		}
	}

	void DrawCircle(uint16 X0, uint16 Y0, uint16 Radius, I_LCD_HAL::Color Color)
	{
		int16 f = 1 - Radius;
		int16 ddF_x = 1;
		int16 ddF_y = -2 * Radius;
		int16 x = 0;
		int16 y = Radius;

		DrawPixel(X0, Y0 + Radius, Color);
		DrawPixel(X0, Y0 - Radius, Color);
		DrawPixel(X0 + Radius, Y0, Color);
		DrawPixel(X0 - Radius, Y0, Color);

		while (x < y)
		{
			if (f >= 0)
			{
				y--;
				ddF_y += 2;
				f += ddF_y;
			}
			x++;
			ddF_x += 2;
			f += ddF_x;

			DrawPixel(X0 + x, Y0 + y, Color);
			DrawPixel(X0 - x, Y0 + y, Color);
			DrawPixel(X0 + x, Y0 - y, Color);
			DrawPixel(X0 - x, Y0 - y, Color);
			DrawPixel(X0 + y, Y0 + x, Color);
			DrawPixel(X0 - y, Y0 + x, Color);
			DrawPixel(X0 + y, Y0 - x, Color);
			DrawPixel(X0 - y, Y0 - x, Color);
		}
	}

	void DrawFilledCircle(uint16 X0, uint16 Y0, uint16 Radius, I_LCD_HAL::Color Color)
	{
		DrawLine(X0, Y0 - Radius, X0, Y0 + Radius + 1, Color);

		const uint8 CORNER_NAME = 3;

		int16 f = 1 - Radius;
		int16 ddF_x = 1;
		int16 ddF_y = -2 * Radius;
		int16 x = 0;
		int16 y = Radius;

		uint16 delta = 0;

		delta++;

		while (x < y)
		{
			if (f >= 0)
			{
				y--;
				ddF_y += 2;
				f += ddF_y;
			}
			x++;
			ddF_x += 2;
			f += ddF_x;

			if (CORNER_NAME & 0x1)
			{
				DrawLine(X0 + x, Y0 - y, X0 + x, Y0 - y + 2 * y + delta - 1, Color);
				DrawLine(X0 + y, Y0 - x, X0 + y, Y0 - x + 2 * x + delta - 1, Color);
			}

			if (CORNER_NAME & 0x2)
			{
				DrawLine(X0 - x, Y0 - y, X0 - x, Y0 - y + 2 * y + delta - 1, Color);
				DrawLine(X0 - y, Y0 - x, X0 - y, Y0 - x + 2 * x + delta - 1, Color);
			}
		}
	}

	void DrawCharacter(uint16 X, uint16 Y, char Char, FontDef Font, I_LCD_HAL::Color Color)
	{
		if (Char < ' ' || '~' < Char)
			return;

		uint8 charIndex = Char - ' ';

		for (uint16 i = 0; i < Font.FontHeight; ++i)
		{
			uint16 data = Font.data[charIndex * Font.FontHeight + i];

			for (uint16 j = 0; j < Font.FontWidth; ++j)
				if ((data << j) & 0x8000)
					DrawPixel(X + j, Y + i, Color);
		}
	}

	void DrawString(uint16 X, uint16 Y, const char *String, FontDef Font, I_LCD_HAL::Color Color)
	{
		DrawString(X, Y, String, std::strlen(String), Font, Color);
	}

	void DrawString(uint16 X, uint16 Y, const char *String, uint16 Length, FontDef Font, I_LCD_HAL::Color Color)
	{
		uint16 x = X;

		for (uint16 i = 0; i < Length; ++i)
		{
			char ch = String[i];

			if (ch == '\n' || ch == '\r')
			{
				Y += Font.FontHeight + m_LineSpacing;
				x = X;
				continue;
			}

			DrawCharacter(x, Y, ch, Font, Color);

			x += Font.FontWidth + m_CharacterSpacing;
		}
	}

	void SetStringSpacing(uint8 Character, uint8 Line)
	{
		m_CharacterSpacing = Character;
		m_LineSpacing = Line;
	}

private:
	void DrawVerticalLine(int16 X, int16 Y, int16 Height, I_LCD_HAL::Color Color)
	{
		for (int16 i = Y; i < Y + Height; ++i)
			DrawPixel(X, i, Color);
	}

	void DrawHorizontalLine(int16 X, int16 Y, int16 Width, I_LCD_HAL::Color Color)
	{
		for (int16 i = X; i < X + Width; ++i)
			DrawPixel(i, Y, Color);
	}

private:
	I_LCD_HAL *m_HAL;
	uint8 m_CharacterSpacing;
	uint8 m_LineSpacing;
};