#pragma once
#ifndef LCD_CANVAS_H
#define LCD_CANVAS_H

#include "I_LCD_HAL.h"
#include "Font.h"
#include "Bitmap.h"
#include "Rect.h"

class LCDCanvas
{
public:
	LCDCanvas(void);

	void Initialize(I_LCD_HAL* HAL);

	void Clear(Color Color);

	void DrawPixel(int16 X, int16 Y, Color Color);

	void DrawLine(int16 X0, int16 Y0, int16 X1, int16 Y1, Color Color, uint8 Thickness = 1);

	void DrawRectangle(int16 X, int16 Y, uint16 Width, uint16 Height, Color Color, uint8 Thickness = 1);

	void DrawFilledRectangle(int16 X, int16 Y, uint16 Width, uint16 Height, Color Color);

	void DrawParallelogram(uint16 LeftTopX, uint16 LeftTopY, uint16 LeftBottomX, uint16 LeftBottomY, uint16 RightTopX, uint16 RightTopY, uint16 RightBottomX, uint16 RightBottomY, Color Color, uint8 Thickness = 1)
	{
		DrawLine(LeftTopX, LeftTopY, LeftBottomX, LeftBottomY, Color, Thickness);
		DrawLine(LeftTopX, LeftTopY, RightTopX, RightTopY, Color, Thickness);
		DrawLine(RightTopX, RightTopY, RightBottomX, RightBottomY, Color, Thickness);
		DrawLine(LeftBottomX, LeftBottomY, RightBottomX, RightBottomY, Color, Thickness);
	}

	void DrawFilledParallelogram(uint16 LeftTopX, uint16 LeftTopY, uint16 LeftBottomX, uint16 LeftBottomY, uint16 RightTopX, uint16 RightTopY, uint16 RightBottomX, uint16 RightBottomY, Color Color);

	void DrawTriangle(int16 X0, int16 Y0, int16 X1, int16 Y1, int16 X2, int16 Y2, Color Color, uint8 Thickness = 1)
	{
		DrawLine(X0, Y0, X1, Y1, Color, Thickness);
		DrawLine(X1, Y1, X2, Y2, Color, Thickness);
		DrawLine(X2, Y2, X0, Y0, Color, Thickness);
	}

	void DrawFilledTriangle(int16 X0, int16 Y0, int16 X1, int16 Y1, int16 X2, int16 Y2, Color Color);

	void DrawCircle(int16 X0, int16 Y0, int16 Radius, Color Color, uint8 Thickness = 1);

	void DrawFilledCircle(int16 X0, int16 Y0, int16 Radius, Color Color);

	uint8 DrawCharacter(int16 X, int16 Y, char Character, const Font& Font, Color Color, bool IgnoreOffset = false);

	void DrawString(int16 X, int16 Y, cstr const String, const Font& Font, Color Color);

	void DrawString(int16 X, int16 Y, cstr String, uint16 Length, const Font& Font, Color Color);

	Point MeasureCharacterDimension(char Character, const Font& Font);

	Point MeasureCharacterOffset(char Character, const Font& Font)
	{
		if (Character == '\n' || Character == '\r')
			return {};

		return GetCharacterOffset(Character, Font);
	}

	Point MeasureStringDimension(cstr String, const Font& Font);

	Point MeasureStringDimension(cstr String, uint16 Length, const Font& Font);

	void DrawBitmap(int16 X, int16 Y, const Bitmap& Bitmap, Color Color, float Scale = 1);

	void DrawFontBitmap(int16 X, int16 Y, uint8 ID, const Font& Font, Color Color, bool IgnoreOffsets = true)
	{
		DrawCharacter(X, Y, ID, Font, Color, IgnoreOffsets);
	}

	Point MeasureFontBitmapDimension(uint8 ID, const Font& Font)
	{
		return GetCharacterDimension(ID, Font, false);
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

	void DrawCharacter(Point Position, char Char, const Font& Font, Color Color)
	{
		DrawCharacter(Position.X, Position.Y, Char, Font, Color);
	}

	void DrawString(Point Position, cstr String, const Font& Font, Color Color)
	{
		DrawString(Position.X, Position.Y, String, Font, Color);
	}

	void DrawString(Point Position, cstr String, uint16 Length, const Font& Font, Color Color)
	{
		DrawString(Position.X, Position.Y, String, Length, Font, Color);
	}

	void DrawBitmap(Point Position, const Bitmap& Bitmap, Color Color, float Scale = 1)
	{
		DrawBitmap(Position.X, Position.Y, Bitmap, Color, Scale);
	}

	void DrawFontBitmap(Point Position, uint8 ID, const Font& Font, Color Color, bool IgnoreOffsets = true)
	{
		DrawFontBitmap(Position.X, Position.Y, ID, Font, Color, IgnoreOffsets);
	}

	void SetStringSpacing(int8 Character, int8 Line)
	{
		m_CharacterSpacing = Character;
		m_LineSpacing = Line;
	}

	const Point& GetDimension(void) const
	{
		return m_HAL->GetDimension();
	}

private:
	bool HasGlyph(char Character, const Font& Font)
	{
		return (GetGlyphIndex(Character, Font) != -1);
	}

	int8 GetGlyphIndex(char Character, const Font& Font);

	Point GetCharacterDimension(char Character, const Font& Font, bool IncludeOffset, bool AdvanceInsteadOfWidth = false);

	Point GetCharacterOffset(char Character, const Font& Font);

	void DrawVerticalLine(int16 X, int16 Y, int16 Height, Color Color, uint8 Thickness = 1);

	void DrawHorizontalLine(int16 X, int16 Y, int16 Width, Color Color, uint8 Thickness = 1);

private:
	I_LCD_HAL* m_HAL;
	int8 m_CharacterSpacing;
	int8 m_LineSpacing;
};

#endif