#pragma once
#ifndef LCD_CANVAS_H
#define LCD_CANVAS_H

#include "I_LCD_HAL.h"
#include "DSP/Math.h"
#include "DSP/Debug.h"

class LCDCanvas
{
#define DEFINE_GLYPH_VALUES()                                                                                                 \
	const uint8 BitsInDataType = sizeof(Font::DataType) * 8;                                                                  \
	const uint8 Pitch = Math::Max(1, Math::Ceil((Font.Height * Font.BitsPerPixel) / (float)BitsInDataType));                  \
	const uint8 CharIndex = GetGlyphIndex(Character, Font);                                                                   \
	const uint8 GlyphDataElementOffset = (Font.HasGlyphData ? 1 : 0);                                                         \
	const Font::DataType *CharData = Font.Data + (CharIndex * (GlyphDataElementOffset + (Font.Height * Pitch)));              \
	uint16 __bitOffset = 0;                                                                                                   \
	const uint8 GlyphAdvance = (Font.HasGlyphData ? uint8((CharData[0] >> __bitOffset) & 0xFF) : Font.MaxWidth) * Font.Scale; \
	__bitOffset += sizeof(uint8) * 8;                                                                                         \
	const int8 GlyphXOffset = (Font.HasGlyphData ? int8((CharData[0] >> __bitOffset) & 0xFF) * Font.Scale : 0);               \
	__bitOffset += sizeof(uint8) * 8;                                                                                         \
	const int8 GlyphYOffset = (Font.HasGlyphData ? int8((CharData[0] >> __bitOffset) & 0xFF) * Font.Scale : 0);               \
	__bitOffset += sizeof(uint8) * 8;                                                                                         \
	const int8 GlyphWidth = (Font.HasGlyphData ? int8((CharData[0] >> __bitOffset) & 0xFF) : Font.MaxWidth) * Font.Scale;     \
	__bitOffset += sizeof(uint8) * 8;                                                                                         \
	const int8 GlyphHeight = (Font.HasGlyphData ? int8((CharData[0] >> __bitOffset) & 0xFF) : Font.Height) * Font.Scale;      \
	__bitOffset += sizeof(uint8) * 8;                                                                                         \
	((void)GlyphAdvance);                                                                                                     \
	((void)GlyphXOffset);                                                                                                     \
	((void)GlyphYOffset);                                                                                                     \
	((void)GlyphWidth);                                                                                                       \
	((void)GlyphHeight);

public:
	LCDCanvas(void)
		: m_HAL(nullptr),
		  m_CharacterSpacing(0),
		  m_LineSpacing(0)
	{
	}

	void Initialize(I_LCD_HAL *HAL)
	{
		ASSERT(HAL != nullptr, "HAL cannot be null");

		m_HAL = HAL;
	}

	void Clear(Color Color)
	{
		ASSERT(m_HAL != nullptr, "m_HAL cannot be null");

		m_HAL->Clear(Color);
	}

	void DrawPixel(int16 X, int16 Y, Color Color)
	{
		ASSERT(m_HAL != nullptr, "m_HAL cannot be null");

		m_HAL->DrawPixel({X, Y}, Color);
	}

	void DrawLine(int16 X0, int16 Y0, int16 X1, int16 Y1, Color Color, uint8 Thickness = 1)
	{
		if (X0 == X1)
		{
			DrawVerticalLine(X0, Y0, Y1 - Y0 + 1, Color, Thickness);

			return;
		}

		if (Y0 == Y1)
		{
			DrawHorizontalLine(X0, Y0, X1 - X0 + 1, Color, Thickness);

			return;
		}

		for (uint8 t = 0; t < Thickness; ++t)
		{
			int16 x0 = (X0 - Thickness / 2) + t;
			int16 x1 = (X1 - Thickness / 2) + t;
			int16 y0 = Y0;

			int32 deltaX = Math::Absolute((int16)x1 - (int16)x0);
			int32 deltaY = Math::Absolute((int16)Y1 - (int16)Y0);
			int32 signX = Math::Sign((int16)x1 - (int16)x0);
			int32 signY = Math::Sign((int16)Y1 - (int16)Y0);
			int32 error = deltaX - deltaY;

			DrawPixel(x1, Y1, Color);

			while ((x0 != x1) || (y0 != Y1))
			{
				DrawPixel(x0, y0, Color);

				int32 error2 = error * 2;

				if (error2 > -deltaY)
				{
					error -= deltaY;
					x0 += signX;
				}

				if (error2 < deltaX)
				{
					error += deltaX;
					y0 += signY;
				}
			}
		}
	}

	void DrawRectangle(int16 X, int16 Y, uint16 Width, uint16 Height, Color Color, uint8 Thickness = 1)
	{
		int16 x2 = X + Width;
		int16 y2 = Y + Height;

		DrawLine(X, Y, X, y2, Color, Thickness);
		DrawLine(X, Y, x2, Y, Color, Thickness);
		DrawLine(X, y2, x2, y2, Color, Thickness);
		DrawLine(x2, Y, x2, y2, Color, Thickness);
	}

	void DrawFilledRectangle(int16 X, int16 Y, uint16 Width, uint16 Height, Color Color)
	{
		for (uint32 j = 0; j < Height; ++j)
			for (uint32 i = 0; i < Width; ++i)
				DrawPixel(X + i, Y + j, Color);
	}

	void DrawParallelogram(uint16 LeftTopX, uint16 LeftTopY, uint16 LeftBottomX, uint16 LeftBottomY, uint16 RightTopX, uint16 RightTopY, uint16 RightBottomX, uint16 RightBottomY, Color Color, uint8 Thickness = 1)
	{
		DrawLine(LeftTopX, LeftTopY, LeftBottomX, LeftBottomY, Color, Thickness);
		DrawLine(LeftTopX, LeftTopY, RightTopX, RightTopY, Color, Thickness);
		DrawLine(RightTopX, RightTopY, RightBottomX, RightBottomY, Color, Thickness);
		DrawLine(LeftBottomX, LeftBottomY, RightBottomX, RightBottomY, Color, Thickness);
	}

	void DrawFilledParallelogram(uint16 LeftTopX, uint16 LeftTopY, uint16 LeftBottomX, uint16 LeftBottomY, uint16 RightTopX, uint16 RightTopY, uint16 RightBottomX, uint16 RightBottomY, Color Color)
	{
		uint16 minX = LeftTopX;
		uint16 minY = LeftTopY;

		uint16 maxX = RightBottomX;
		uint16 maxY = RightBottomY;

		// TODO: Handle the middle point
		if (LeftTopX != LeftBottomX)
		{
			minX = Math::Max(LeftTopX, LeftBottomX);

			DrawFilledTriangle(LeftTopX, LeftTopY, LeftTopX, LeftBottomY, LeftBottomX, LeftBottomY, Color);
		}

		// TODO: Handle the middle point
		if (LeftTopY != RightTopY)
		{
			minY = Math::Max(LeftTopY, RightTopY);

			DrawFilledTriangle(LeftTopX, LeftTopY, LeftTopX, RightTopY, RightBottomX, RightBottomY, Color);
		}

		if (RightTopX != RightBottomX)
		{
			maxX = Math::Min(RightTopX, RightBottomX);

			uint16 y2 = RightBottomY;
			if (RightTopX > RightBottomX)
				y2 = RightTopY;

			DrawFilledTriangle(RightTopX, RightTopY, maxX, y2, RightBottomX, RightBottomY, Color);
		}

		// TODO: Handle the middle point
		if (LeftBottomY != RightBottomY)
		{
			maxY = Math::Min(LeftBottomY, RightBottomY);

			DrawFilledTriangle(LeftBottomX, LeftBottomY, LeftBottomX, RightBottomY, RightBottomX, RightBottomY, Color);
		}

		DrawFilledRectangle(minX, minY, maxX - minX, maxY - minY, Color);
	}

	void DrawTriangle(int16 X0, int16 Y0, int16 X1, int16 Y1, int16 X2, int16 Y2, Color Color, uint8 Thickness = 1)
	{
		DrawLine(X0, Y0, X1, Y1, Color, Thickness);
		DrawLine(X1, Y1, X2, Y2, Color, Thickness);
		DrawLine(X2, Y2, X0, Y0, Color, Thickness);
	}

	void DrawFilledTriangle(int16 X0, int16 Y0, int16 X1, int16 Y1, int16 X2, int16 Y2, Color Color)
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

		for (y = Y0; y < last; ++y)
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
		for (; y < Y2; ++y)
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

	void DrawCircle(int16 X0, int16 Y0, int16 Radius, Color Color, uint8 Thickness = 1)
	{
		--Radius;

		for (int16 r = -Thickness / 2; r < Thickness; ++r)
		{
			int16 radius = Radius + r;

			int16 f = 1 - radius;
			int16 ddF_x = 1;
			int16 ddF_y = -2 * radius;
			int16 x = 0;
			int16 y = radius;

			DrawPixel(X0, Y0 + radius, Color);
			DrawPixel(X0, Y0 - radius, Color);
			DrawPixel(X0 + radius, Y0, Color);
			DrawPixel(X0 - radius, Y0, Color);

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
	}

	void DrawFilledCircle(int16 X0, int16 Y0, int16 Radius, Color Color)
	{
		--Radius;

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

	uint8 DrawCharacter(int16 X, int16 Y, char Character, const Font &Font, Color Color, bool IgnoreOffset = false)
	{
		if (!HasGlyph(Character, Font))
			return 0;

		const uint8 PIXEL_ALPHA_VALUES[] = {0, 255, 85, 170};

		DEFINE_GLYPH_VALUES()

		const uint8 Mask = (1 << Font.BitsPerPixel) - 1;
		const uint16 TargetWidth = GlyphWidth;
		const uint16 TargetHeight = GlyphHeight;
		CharData += GlyphDataElementOffset;

		::Color color = {Color.R, Color.G, Color.B};

		int16 originX = X + (IgnoreOffset ? 0 : GlyphXOffset);
		int16 originY = Y + (IgnoreOffset ? 0 : GlyphYOffset);

		for (uint16 ty = 0; ty < TargetHeight; ++ty)
			for (uint16 tx = 0; tx < TargetWidth; ++tx)
			{
				uint8 x = tx / Font.Scale;
				uint8 y = ty / Font.Scale;

				uint8 dataOffset = (x * Font.BitsPerPixel) / BitsInDataType;
				Font::DataType data = CharData[dataOffset + (y * Pitch)];

				uint8 bitIndex = (x * Font.BitsPerPixel) % BitsInDataType;
				uint8 value = PIXEL_ALPHA_VALUES[(data >> bitIndex) & Mask];
				if (value == 0)
					continue;

				color.A = Color::CombineValues(value, Color.A);

				DrawPixel(originX + tx, originY + ty, color);
			}

		return GlyphAdvance;
	}

	void DrawString(int16 X, int16 Y, cstr const String, const Font &Font, Color Color)
	{
		DrawString(X, Y, String, GetStringLength(String), Font, Color);
	}

	void DrawString(int16 X, int16 Y, cstr String, uint16 Length, const Font &Font, Color Color)
	{
		ASSERT(String != nullptr, "String cannot be null");

		if (Length == 0)
			return;

		uint16 x = X;

		for (uint16 i = 0; i < Length; ++i)
		{
			char ch = String[i];

			if (ch == '\n' || ch == '\r')
			{
				Y += (Font.Height * Font.Scale) + m_LineSpacing;
				x = X;
				continue;
			}

			uint8 advance = DrawCharacter(x, Y, ch, Font, Color);

			x += advance + (m_CharacterSpacing * Font.Scale);
		}
	}

	Point MeasureCharacterDimension(char Character, const Font &Font)
	{
		if (Character == '\n' || Character == '\r')
			return {};

		Point dimensions = GetCharacterDimesion(Character, Font, true, true);

		return {dimensions.X + (m_CharacterSpacing * Font.Scale), (dimensions.Y * Font.Scale) + m_LineSpacing};
	}

	Point MeasureCharacterOffset(char Character, const Font &Font)
	{
		if (Character == '\n' || Character == '\r')
			return {};

		return GetCharacterOffset(Character, Font);
	}

	Point MeasureStringDimension(cstr String, const Font &Font)
	{
		return MeasureStringDimension(String, GetStringLength(String), Font);
	}

	Point MeasureStringDimension(cstr String, uint16 Length, const Font &Font)
	{
		ASSERT(String != nullptr, "String cannot be null");

		if (Length == 0)
			return {};

		Point totalDimensions = {};
		Point lineDimensions = {};
		for (uint16 i = 0; i < Length; ++i)
		{
			char ch = String[i];

			Point dimensions = GetCharacterDimesion(ch, Font, true, true);

			lineDimensions.X += dimensions.X + m_CharacterSpacing;

			uint16 height = dimensions.Y + m_LineSpacing;
			if (lineDimensions.Y < height)
				lineDimensions.Y = height;

			if (totalDimensions.X < lineDimensions.X)
				totalDimensions.X = lineDimensions.X;

			if (ch == '\n' || ch == '\r')
			{
				totalDimensions.Y += lineDimensions.Y;
				lineDimensions = {};

				continue;
			}
		}

		totalDimensions.Y += lineDimensions.Y;

		return totalDimensions;
	}

	void DrawBitmap(int16 X, int16 Y, const Bitmap &Bitmap, Color Color, float Scale = 1)
	{
		const uint8 CHANNEL_VALUES[] = {0, 255, 85, 170};

		const uint8 BitsInDataType = sizeof(Bitmap::DataType) * 8;
		const uint8 Pitch = Math::Max(1, Math::Ceil(((float)Bitmap.Width * Bitmap.BitsPerPixel) / BitsInDataType));

		const uint8 BitsPerValue = Math::Min(2, Bitmap.BitsPerPixel);
		const uint8 Mask = (1 << BitsPerValue) - 1;
		const uint16 TargetWidth = Bitmap.Width * Scale;
		const uint16 TargetHeight = Bitmap.Height * Scale;

		for (uint16 ty = 0; ty < TargetHeight; ++ty)
			for (uint16 tx = 0; tx < TargetWidth; ++tx)
			{
				uint8 x = tx / Scale;
				uint8 y = ty / Scale;

				uint8 dataOffset = (x * Bitmap.BitsPerPixel) / BitsInDataType;
				Font::DataType data = Bitmap.Data[dataOffset + (y * Pitch)];

				uint8 bitIndex = (x * Bitmap.BitsPerPixel) % BitsInDataType;
				uint8 value = CHANNEL_VALUES[(data >> bitIndex) & Mask];
				if (value == 0)
					continue;

				::Color color = Color;

				color.A = Color::CombineValues(value, color.A);

				if (Bitmap.BitsPerPixel == 4)
				{
					bitIndex += BitsPerValue;
					value = CHANNEL_VALUES[(data >> bitIndex) & Mask];

					color *= value;
				}

				DrawPixel(X + tx, Y + ty, color);
			}
	}

	void DrawFontBitmap(int16 X, int16 Y, uint8 ID, const Font &Font, Color Color, bool IgnoreOffsets = true)
	{
		DrawCharacter(X, Y, ID, Font, Color, IgnoreOffsets);
	}

	Point MeasureFontBitmapDimension(uint8 ID, const Font &Font)
	{
		return GetCharacterDimesion(ID, Font, false);
	}

	void DrawPixel(Point Position, Color Color)
	{
		DrawPixel(Position.X, Position.Y, Color);
	}

	void DrawLine(Point Position0, Point Position1, Color Color, uint8 Thickness = 1)
	{
		DrawLine(Position0.X, Position0.Y, Position1.X, Position1.Y, Color, Thickness);
	}

	void DrawRectangle(Rect Rect, Color Color, uint8 Thickness = 1)
	{
		DrawRectangle(Rect.Position.X, Rect.Position.Y, Rect.Dimension.X, Rect.Dimension.Y, Color, Thickness);
	}

	void DrawFilledRectangle(Rect Rect, Color Color)
	{
		DrawFilledRectangle(Rect.Position.X, Rect.Position.Y, Rect.Dimension.X, Rect.Dimension.Y, Color);
	}

	void DrawParallelogram(Point LeftTop, Point LeftBottom, Point RightTop, Point RightBottom, Color Color, uint8 Thickness = 1)
	{
		DrawParallelogram(LeftTop.X, LeftTop.Y, LeftBottom.X, LeftBottom.Y, RightTop.X, RightTop.Y, RightBottom.X, RightBottom.Y, Color, Thickness);
	}

	void DrawFilledParallelogram(Point LeftTop, Point LeftBottom, Point RightTop, Point RightBottom, Color Color)
	{
		DrawFilledParallelogram(LeftTop.X, LeftTop.Y, LeftBottom.X, LeftBottom.Y, RightTop.X, RightTop.Y, RightBottom.X, RightBottom.Y, Color);
	}

	void DrawTriangle(Point Position0, Point Position1, Point Position2, Color Color, uint8 Thickness = 1)
	{
		DrawTriangle(Position0.X, Position0.Y, Position1.X, Position1.Y, Position2.X, Position2.Y, Color, Thickness);
	}

	void DrawFilledTriangle(Point Position0, Point Position1, Point Position2, Color Color)
	{
		DrawFilledTriangle(Position0.X, Position0.Y, Position1.X, Position1.Y, Position2.X, Position2.Y, Color);
	}

	void DrawCircle(Point Position, int16 Radius, Color Color, uint8 Thickness = 1)
	{
		DrawCircle(Position.X, Position.Y, Radius, Color, Thickness);
	}

	void DrawFilledCircle(Point Position, int16 Radius, Color Color)
	{
		DrawFilledCircle(Position.X, Position.Y, Radius, Color);
	}

	void DrawCharacter(Point Position, char Char, const Font &Font, Color Color)
	{
		DrawCharacter(Position.X, Position.Y, Char, Font, Color);
	}

	void DrawString(Point Position, cstr String, const Font &Font, Color Color)
	{
		DrawString(Position.X, Position.Y, String, Font, Color);
	}

	void DrawString(Point Position, cstr String, uint16 Length, const Font &Font, Color Color)
	{
		DrawString(Position.X, Position.Y, String, Length, Font, Color);
	}

	void DrawBitmap(Point Position, const Bitmap &Bitmap, Color Color, float Scale = 1)
	{
		DrawBitmap(Position.X, Position.Y, Bitmap, Color, Scale);
	}

	void DrawFontBitmap(Point Position, uint8 ID, const Font &Font, Color Color, bool IgnoreOffsets = true)
	{
		DrawFontBitmap(Position.X, Position.Y, ID, Font, Color, IgnoreOffsets);
	}

	void SetStringSpacing(int8 Character, int8 Line)
	{
		m_CharacterSpacing = Character;
		m_LineSpacing = Line;
	}

	const Point &GetDimension(void) const
	{
		return m_HAL->GetDimension();
	}

private:
	bool HasGlyph(char Character, const Font &Font)
	{
		return (GetGlyphIndex(Character, Font) != -1);
	}

	int8 GetGlyphIndex(char Character, const Font &Font)
	{
		for (int8 i = 0;; ++i)
		{
			char c = Font.Glyphs[i];
			if (c == '\0')
				break;

			if (c == Character)
				return i;
		}

		return -1;
	}

	Point GetCharacterDimesion(char Character, const Font &Font, bool IncludeOffset, bool AdvanceInsteadOfWidth = false)
	{
		if (!HasGlyph(Character, Font))
			return {};

		uint8 width = Font.MaxWidth * Font.Scale;
		uint8 height = Font.Height * Font.Scale;

		if (Font.HasGlyphData)
		{
			DEFINE_GLYPH_VALUES()

			width = (AdvanceInsteadOfWidth ? GlyphAdvance : GlyphWidth);
			height = GlyphHeight;

			if (IncludeOffset)
			{
				width += GlyphXOffset;
				height += GlyphYOffset;
			}
		}

		return {width, height};
	}

	Point GetCharacterOffset(char Character, const Font &Font)
	{
		if (!HasGlyph(Character, Font))
			return {};

		if (Font.HasGlyphData)
		{
			DEFINE_GLYPH_VALUES()

			return {GlyphXOffset, GlyphYOffset};
		}

		return {};
	}

	void DrawVerticalLine(int16 X, int16 Y, int16 Height, Color Color, uint8 Thickness = 1)
	{
		int16 x = X - (Thickness / 2);

		if (Height < 0)
		{
			Y = Math::Max(0, Y + Height);
			Height *= -1;
		}

		for (uint8 tX = 0; tX < Thickness; ++tX)
			for (uint16 i = Y; i < Y + Height; ++i)
				DrawPixel(x + tX, i, Color);
	}

	void DrawHorizontalLine(int16 X, int16 Y, int16 Width, Color Color, uint8 Thickness = 1)
	{
		int16 y = Y - (Thickness / 2);

		if (Width < 0)
		{
			X = Math::Max(0, X + Width);
			Width *= -1;
		}

		for (uint8 tY = 0; tY < Thickness; ++tY)
			for (uint16 i = X; i < X + Width; ++i)
				DrawPixel(i, y + tY, Color);
	}

private:
	I_LCD_HAL *m_HAL;
	int8 m_CharacterSpacing;
	int8 m_LineSpacing;

#undef DEFINE_GLYPH_VALUES
};

#endif