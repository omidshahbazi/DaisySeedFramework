#pragma once

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

	struct Color
	{
	public:
		Color(void)
			: R(0),
			  G(0),
			  B(0),
			  A(255)
		{
		}

		Color(uint8 R, uint8 G, uint8 B)
			: R(R),
			  G(G),
			  B(B),
			  A(255)
		{
		}

		Color(uint8 R, uint8 G, uint8 B, uint8 A)
			: R(R),
			  G(G),
			  B(B),
			  A(A)
		{
		}

		uint16 R5G6B5(void)
		{
			uint16 b = (B >> 3) & 0x1f;
			uint16 g = ((G >> 2) & 0x3f) << 5;
			uint16 r = ((R >> 3) & 0x1f) << 11;

			return (uint16)(r | g | b);
		}

		uint8 R;
		uint8 G;
		uint8 B;
		uint8 A;
	};

public:
	virtual void Update(void) = 0;

	virtual void Clear(Color Color) = 0;

	virtual void DrawPixel(uint16 X, uint16 Y, Color Color) = 0;
};