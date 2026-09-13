#include "DaisySeedFramework/LCDCanvas.h"
#include "DaisySeedFramework/StringUtils.h"
#include <DigitalSignalProcessing/Math.h>
#include <DigitalSignalProcessing/Debug.h>

#define DEFINE_GLYPH_VALUES()                                                                                                 \
	const uint8_t BitsInDataType = sizeof(Font::DataType) * 8;                                                                  \
	const uint8_t Pitch = Math::Max(1, Math::Ceil((Font.Height * Font.BitsPerPixel) / (float)BitsInDataType));                  \
	const uint8_t CharIndex = GetGlyphIndex(Character, Font);                                                                   \
	const uint8_t GlyphDataElementOffset = (Font.HasGlyphData ? 1 : 0);                                                         \
	const Font::DataType *CharData = Font.Data + (CharIndex * (GlyphDataElementOffset + (Font.Height * Pitch)));              \
	uint16_t __bitOffset = 0;                                                                                                   \
	const uint8_t GlyphAdvance = (Font.HasGlyphData ? uint8_t((CharData[0] >> __bitOffset) & 0xFF) : Font.MaxWidth) * Font.Scale; \
	__bitOffset += sizeof(uint8_t) * 8;                                                                                         \
	const int8_t GlyphXOffset = (Font.HasGlyphData ? int8_t((CharData[0] >> __bitOffset) & 0xFF) * Font.Scale : 0);               \
	__bitOffset += sizeof(uint8_t) * 8;                                                                                         \
	const int8_t GlyphYOffset = (Font.HasGlyphData ? int8_t((CharData[0] >> __bitOffset) & 0xFF) * Font.Scale : 0);               \
	__bitOffset += sizeof(uint8_t) * 8;                                                                                         \
	const int8_t GlyphWidth = (Font.HasGlyphData ? int8_t((CharData[0] >> __bitOffset) & 0xFF) : Font.MaxWidth) * Font.Scale;     \
	__bitOffset += sizeof(uint8_t) * 8;                                                                                         \
	const int8_t GlyphHeight = (Font.HasGlyphData ? int8_t((CharData[0] >> __bitOffset) & 0xFF) : Font.Height) * Font.Scale;      \
	__bitOffset += sizeof(uint8_t) * 8;                                                                                         \
	((void)GlyphAdvance);                                                                                                     \
	((void)GlyphXOffset);                                                                                                     \
	((void)GlyphYOffset);                                                                                                     \
	((void)GlyphWidth);                                                                                                       \
	((void)GlyphHeight);

LCDCanvas::LCDCanvas(void)
	: m_HAL(nullptr),
	m_CharacterSpacing(0),
	m_LineSpacing(0)
{}

void LCDCanvas::Initialize(I_LCD_HAL* HAL)
{
	ASSERT(HAL != nullptr, "HAL cannot be null");

	m_HAL = HAL;
}

void LCDCanvas::Clear(Color Color)
{
	ASSERT(m_HAL != nullptr, "m_HAL cannot be null");

	m_HAL->Clear(Color);
}

void LCDCanvas::DrawPixel(int16_t X, int16_t Y, Color Color)
{
	ASSERT(m_HAL != nullptr, "m_HAL cannot be null");

	m_HAL->DrawPixel({ X, Y }, Color);
}

void LCDCanvas::DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1, Color Color, uint8_t Thickness)
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

	for (uint8_t t = 0; t < Thickness; ++t)
	{
		int16_t x0 = (X0 - Thickness / 2) + t;
		int16_t x1 = (X1 - Thickness / 2) + t;
		int16_t y0 = Y0;

		int32_t deltaX = Math::Absolute((int16_t)x1 - (int16_t)x0);
		int32_t deltaY = Math::Absolute((int16_t)Y1 - (int16_t)Y0);
		int32_t signX = Math::Sign((int16_t)x1 - (int16_t)x0);
		int32_t signY = Math::Sign((int16_t)Y1 - (int16_t)Y0);
		int32_t error = deltaX - deltaY;

		DrawPixel(x1, Y1, Color);

		while ((x0 != x1) || (y0 != Y1))
		{
			DrawPixel(x0, y0, Color);

			int32_t error2 = error * 2;

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

void LCDCanvas::DrawRectangle(int16_t X, int16_t Y, uint16_t Width, uint16_t Height, Color Color, uint8_t Thickness)
{
	int16_t x2 = X + Width;
	int16_t y2 = Y + Height;

	DrawLine(X, Y, X, y2, Color, Thickness);
	DrawLine(X, Y, x2, Y, Color, Thickness);
	DrawLine(X, y2, x2, y2, Color, Thickness);
	DrawLine(x2, Y, x2, y2, Color, Thickness);
}

void LCDCanvas::DrawFilledRectangle(int16_t X, int16_t Y, uint16_t Width, uint16_t Height, Color Color)
{
	for (uint32_t j = 0; j < Height; ++j)
		for (uint32_t i = 0; i < Width; ++i)
			DrawPixel(X + i, Y + j, Color);
}

void LCDCanvas::DrawFilledParallelogram(uint16_t LeftTopX, uint16_t LeftTopY, uint16_t LeftBottomX, uint16_t LeftBottomY, uint16_t RightTopX, uint16_t RightTopY, uint16_t RightBottomX, uint16_t RightBottomY, Color Color)
{
	uint16_t minX = LeftTopX;
	uint16_t minY = LeftTopY;

	uint16_t maxX = RightBottomX;
	uint16_t maxY = RightBottomY;

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

		uint16_t y2 = RightBottomY;
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

void LCDCanvas::DrawFilledTriangle(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, Color Color)
{
	int16_t a, b, y, last;

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

	int16_t dx01 = X1 - X0, dy01 = Y1 - Y0, dx02 = X2 - X0, dy02 = Y2 - Y0,
		dx12 = X2 - X1, dy12 = Y2 - Y1;
	int32_t sa = 0, sb = 0;

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
	sa = (int32_t)dx12 * (y - Y1);
	sb = (int32_t)dx02 * (y - Y0);
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

void LCDCanvas::DrawCircle(int16_t X0, int16_t Y0, int16_t Radius, Color Color, uint8_t Thickness)
{
	--Radius;

	for (int16_t r = -Thickness / 2; r < Thickness; ++r)
	{
		int16_t radius = Radius + r;

		int16_t f = 1 - radius;
		int16_t ddF_x = 1;
		int16_t ddF_y = -2 * radius;
		int16_t x = 0;
		int16_t y = radius;

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

void LCDCanvas::DrawFilledCircle(int16_t X0, int16_t Y0, int16_t Radius, Color Color)
{
	--Radius;

	DrawLine(X0, Y0 - Radius, X0, Y0 + Radius + 1, Color);

	const uint8_t CORNER_NAME = 3;

	int16_t f = 1 - Radius;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * Radius;
	int16_t x = 0;
	int16_t y = Radius;

	uint16_t delta = 0;

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

uint8_t LCDCanvas::DrawCharacter(int16_t X, int16_t Y, char Character, const Font& Font, Color Color, bool IgnoreOffset)
{
	if (!HasGlyph(Character, Font))
		return 0;

	const uint8_t PIXEL_ALPHA_VALUES[] = { 0, 255, 85, 170 };

	DEFINE_GLYPH_VALUES()

		const uint8_t Mask = (1 << Font.BitsPerPixel) - 1;
	const uint16_t TargetWidth = GlyphWidth;
	const uint16_t TargetHeight = GlyphHeight;
	CharData += GlyphDataElementOffset;

	::Color color = { Color.R, Color.G, Color.B };

	int16_t originX = X + (IgnoreOffset ? 0 : GlyphXOffset);
	int16_t originY = Y + (IgnoreOffset ? 0 : GlyphYOffset);

	for (uint16_t ty = 0; ty < TargetHeight; ++ty)
		for (uint16_t tx = 0; tx < TargetWidth; ++tx)
		{
			uint8_t x = tx / Font.Scale;
			uint8_t y = ty / Font.Scale;

			uint8_t dataOffset = (x * Font.BitsPerPixel) / BitsInDataType;
			Font::DataType data = CharData[dataOffset + (y * Pitch)];

			uint8_t bitIndex = (x * Font.BitsPerPixel) % BitsInDataType;
			uint8_t value = PIXEL_ALPHA_VALUES[(data >> bitIndex) & Mask];
			if (value == 0)
				continue;

			color.A = Color::CombineValues(value, Color.A);

			DrawPixel(originX + tx, originY + ty, color);
		}

	return GlyphAdvance;
}

void LCDCanvas::DrawString(int16_t X, int16_t Y, cstr const String, const Font& Font, Color Color)
{
	DrawString(X, Y, String, GetStringLength(String), Font, Color);
}

void LCDCanvas::DrawString(int16_t X, int16_t Y, cstr String, uint16_t Length, const Font& Font, Color Color)
{
	ASSERT(String != nullptr, "String cannot be null");

	if (Length == 0)
		return;

	uint16_t x = X;

	for (uint16_t i = 0; i < Length; ++i)
	{
		char ch = String[i];

		if (ch == '\n' || ch == '\r')
		{
			Y += Font.GetScaledSize().Y + m_LineSpacing;
			x = X;
			continue;
		}

		uint8_t advance = DrawCharacter(x, Y, ch, Font, Color);

		x += advance + (m_CharacterSpacing * Font.Scale);
	}
}

Point LCDCanvas::MeasureCharacterDimension(char Character, const Font& Font)
{
	if (Character == '\n' || Character == '\r')
		return {};

	Point dimensions = GetCharacterDimension(Character, Font, true, true);

	return { dimensions.X + (m_CharacterSpacing * Font.Scale), (dimensions.Y * Font.Scale) + m_LineSpacing };
}

Point LCDCanvas::MeasureStringDimension(cstr String, const Font& Font)
{
	return MeasureStringDimension(String, GetStringLength(String), Font);
}

Point LCDCanvas::MeasureStringDimension(cstr String, uint16_t Length, const Font& Font)
{
	ASSERT(String != nullptr, "String cannot be null");

	if (Length == 0)
		return {};

	Point totalDimensions = {};
	Point lineDimensions = {};
	for (uint16_t i = 0; i < Length; ++i)
	{
		char ch = String[i];

		Point dimensions = GetCharacterDimension(ch, Font, true, true);

		lineDimensions.X += dimensions.X + m_CharacterSpacing;

		uint16_t height = dimensions.Y + m_LineSpacing;
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

void LCDCanvas::DrawBitmap(int16_t X, int16_t Y, const Bitmap& Bitmap, Color Color, float Scale)
{
	const uint8_t CHANNEL_VALUES[] = { 0, 255, 85, 170 };

	const uint8_t BitsInDataType = sizeof(Bitmap::DataType) * 8;
	const uint8_t Pitch = Math::Max(1, Math::Ceil(((float)Bitmap.Width * Bitmap.BitsPerPixel) / BitsInDataType));

	const uint8_t BitsPerValue = Math::Min(2, Bitmap.BitsPerPixel);
	const uint8_t Mask = (1 << BitsPerValue) - 1;
	const uint16_t TargetWidth = Bitmap.Width * Scale;
	const uint16_t TargetHeight = Bitmap.Height * Scale;

	for (uint16_t ty = 0; ty < TargetHeight; ++ty)
		for (uint16_t tx = 0; tx < TargetWidth; ++tx)
		{
			uint8_t x = tx / Scale;
			uint8_t y = ty / Scale;

			uint8_t dataOffset = (x * Bitmap.BitsPerPixel) / BitsInDataType;
			Font::DataType data = Bitmap.Data[dataOffset + (y * Pitch)];

			uint8_t bitIndex = (x * Bitmap.BitsPerPixel) % BitsInDataType;
			uint8_t value = CHANNEL_VALUES[(data >> bitIndex) & Mask];
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

int8_t LCDCanvas::GetGlyphIndex(char Character, const Font& Font)
{
	for (int8_t i = 0;; ++i)
	{
		char c = Font.Glyphs[i];
		if (c == '\0')
			break;

		if (c == Character)
			return i;
	}

	return -1;
}

Point LCDCanvas::GetCharacterDimension(char Character, const Font& Font, bool IncludeOffset, bool AdvanceInsteadOfWidth)
{
	if (!HasGlyph(Character, Font))
		return {};

	Point fontSize = Font.GetScaledSize();

	if (Font.HasGlyphData)
	{
		DEFINE_GLYPH_VALUES()

			fontSize.X = (AdvanceInsteadOfWidth ? GlyphAdvance : GlyphWidth);
		fontSize.Y = GlyphHeight;

		if (IncludeOffset)
		{
			fontSize.X += GlyphXOffset;
			fontSize.Y += GlyphYOffset;
		}
	}

	return fontSize;
}

Point LCDCanvas::GetCharacterOffset(char Character, const Font& Font)
{
	if (!HasGlyph(Character, Font))
		return {};

	if (Font.HasGlyphData)
	{
		DEFINE_GLYPH_VALUES()

			return { GlyphXOffset, GlyphYOffset };
	}

	return {};
}

void LCDCanvas::DrawVerticalLine(int16_t X, int16_t Y, int16_t Height, Color Color, uint8_t Thickness)
{
	int16_t x = X - (Thickness / 2);

	if (Height < 0)
	{
		Y = Math::Max(0, Y + Height);
		Height *= -1;
	}

	for (uint8_t tX = 0; tX < Thickness; ++tX)
		for (uint16_t i = Y; i < Y + Height; ++i)
			DrawPixel(x + tX, i, Color);
}

void LCDCanvas::DrawHorizontalLine(int16_t X, int16_t Y, int16_t Width, Color Color, uint8_t Thickness)
{
	int16_t y = Y - (Thickness / 2);

	if (Width < 0)
	{
		X = Math::Max(0, X + Width);
		Width *= -1;
	}

	for (uint8_t tY = 0; tY < Thickness; ++tY)
		for (uint16_t i = X; i < X + Width; ++i)
			DrawPixel(i, y + tY, Color);
}

#undef DEFINE_GLYPH_VALUES