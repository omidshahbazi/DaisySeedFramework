#pragma once

#include "DSP/Common.h"

enum class GPIOPins
{
	Pin0 = 0, // Digital
	Pin1,	  // Digital
	Pin2,	  // Digital
	Pin3,	  // Digital
	Pin4,	  // Digital
	Pin5,	  // Digital
	Pin6,	  // Digital
	Pin7,	  // Digital
	Pin8,	  // Digital
	Pin9,	  // Digital
	Pin10,	  // Digital
	Pin11,	  // Digital
	Pin12,	  // Digital
	Pin13,	  // Digital
	Pin14,	  // Digital
	Pin15,	  // Analog/Digital
	Pin16,	  // Analog/Digital
	Pin17,	  // Analog/Digital
	Pin18,	  // Analog/Digital
	Pin19,	  // Analog/Digital
	Pin20,	  // Analog/Digital
	Pin21,	  // Analog/Digital
	Pin22,	  // Analog/Digital
	Pin23,	  // Analog/Digital
	Pin24,	  // Analog/Digital
	Pin25,	  // Analog/Digital
	Pin26,	  // Digital
	Pin27,	  // Digital
	Pin28,	  // Analog/Digital
	Pin29,	  // Digital
	Pin30,	  // Digital
	COUNT
};

enum class AnalogPins
{
	Pin0 = (uint8)GPIOPins::Pin15,
	Pin1 = (uint8)GPIOPins::Pin16,
	Pin2 = (uint8)GPIOPins::Pin17,
	Pin3 = (uint8)GPIOPins::Pin18,
	Pin4 = (uint8)GPIOPins::Pin19,
	Pin5 = (uint8)GPIOPins::Pin20,
	Pin6 = (uint8)GPIOPins::Pin21,
	Pin7 = (uint8)GPIOPins::Pin22,
	Pin8 = (uint8)GPIOPins::Pin23,
	Pin9 = (uint8)GPIOPins::Pin24,
	Pin10 = (uint8)GPIOPins::Pin25,
	Pin11 = (uint8)GPIOPins::Pin28
};

#define ANALOG_PIN_COUNT 12

struct Point
{
public:
	Point(void)
		: X(0),
		  Y(0)
	{
	}

	template <typename T, typename U>
	Point(T X, U Y)
		: X(static_cast<int16>(X)),
		  Y(static_cast<int16>(Y))
	{
	}

	Point operator+(Point Other) const
	{
		Point point = *this;
		point += Other;
		return point;
	}

	template <typename T>
	Point operator+(T Value) const
	{
		Point point = *this;
		point += Value;
		return point;
	}

	Point operator-(Point Other) const
	{
		Point point = *this;
		point -= Other;
		return point;
	}

	template <typename T>
	Point operator-(T Value) const
	{
		Point point = *this;
		point -= Value;
		return point;
	}

	Point operator*(Point Other) const
	{
		Point point = *this;
		point *= Other;
		return point;
	}

	template <typename T>
	Point operator*(T Value) const
	{
		Point point = *this;
		point *= Value;
		return point;
	}

	Point operator/(Point Other) const
	{
		Point point = *this;
		point /= Other;
		return point;
	}

	template <typename T>
	Point operator/(T Value) const
	{
		Point point = *this;
		point /= Value;
		return point;
	}

	Point &operator+=(Point Other)
	{
		X += Other.X;
		Y += Other.Y;
		return *this;
	}

	template <typename T>
	Point &operator+=(T Value)
	{
		X += Value;
		Y += Value;
		return *this;
	}

	Point &operator-=(Point Other)
	{
		X -= Other.X;
		Y -= Other.Y;
		return *this;
	}

	template <typename T>
	Point &operator-=(T Value)
	{
		X -= Value;
		Y -= Value;
		return *this;
	}

	Point &operator*=(Point Other)
	{
		X *= Other.X;
		Y *= Other.Y;
		return *this;
	}

	template <typename T>
	Point &operator*=(T Value)
	{
		X *= Value;
		Y *= Value;
		return *this;
	}

	Point &operator/=(Point Other)
	{
		X /= Other.X;
		Y /= Other.Y;
		return *this;
	}

	template <typename T>
	Point &operator/=(T Value)
	{
		X /= Value;
		Y /= Value;
		return *this;
	}

public:
	int16 X;
	int16 Y;
};

struct Rect
{
public:
	Rect(void)
	{
	}

	Rect(Point Position, Point Dimension)
		: Position(Position),
		  Dimension(Dimension)
	{
	}

	Rect(int16 X, int16 Y, int16 Width, int16 Height)
		: Position(X, Y),
		  Dimension(Width, Height)
	{
	}

public:
	Point Position;
	Point Dimension;
};

struct Font
{
public:
	typedef uint64 DataType;

public:
	uint8 MaxWidth;
	uint8 Height;
	const DataType *const Data;
	float Scale;
	uint8 BitsPerPixel;
	bool HasGlyphData;
	cstr Glyphs;

public:
	static constexpr Font CreateScaled(const Font &ReferenceFont, uint8 TargetHeight)
	{
		Font font = ReferenceFont;
		font.Scale = (float)TargetHeight / ReferenceFont.Height;
		return font;
	}
};

struct Bitmap
{
public:
	typedef uint64 DataType;

public:
	uint8 Width;
	uint8 Height;
	const DataType *const Data;
	uint8 BitsPerPixel;
};

#include "libDaisy/src/dev/sdram.h"

#define DEFINE_LARGE_MEMORY_BUFFER(Name, Size)  \
	static constexpr uint32 Name##_Size = Size; \
	uint8 DSY_SDRAM_BSS g_##Name[Name##_Size];