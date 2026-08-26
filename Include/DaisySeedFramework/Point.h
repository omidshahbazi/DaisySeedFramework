#pragma once
#ifndef POINT_H
#define POINT_H

#include <DigitalSignalProcessing/DataTypes.h>

struct Point
{
public:
	Point(void)
		: X(0),
		Y(0)
	{}

	template <typename T, typename U>
	Point(T X, U Y)
		: X(static_cast<int16>(X)),
		Y(static_cast<int16>(Y))
	{}

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

	Point& operator+=(Point Other)
	{
		X += Other.X;
		Y += Other.Y;
		return *this;
	}

	template <typename T>
	Point& operator+=(T Value)
	{
		X += Value;
		Y += Value;
		return *this;
	}

	Point& operator-=(Point Other)
	{
		X -= Other.X;
		Y -= Other.Y;
		return *this;
	}

	template <typename T>
	Point& operator-=(T Value)
	{
		X -= Value;
		Y -= Value;
		return *this;
	}

	Point& operator*=(Point Other)
	{
		X *= Other.X;
		Y *= Other.Y;
		return *this;
	}

	template <typename T>
	Point& operator*=(T Value)
	{
		X *= Value;
		Y *= Value;
		return *this;
	}

	Point& operator/=(Point Other)
	{
		X /= Other.X;
		Y /= Other.Y;
		return *this;
	}

	template <typename T>
	Point& operator/=(T Value)
	{
		X /= Value;
		Y /= Value;
		return *this;
	}

public:
	int16 X;
	int16 Y;
};

#endif