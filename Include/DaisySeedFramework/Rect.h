#pragma once
#ifndef RECT_H
#define RECT_H

#include "Point.h"

struct Rect
{
public:
	Rect(void)
	{}

	Rect(Point Position, Point Dimension)
		: Position(Position),
		Dimension(Dimension)
	{}

	Rect(int16_t X, int16_t Y, int16_t Width, int16_t Height)
		: Position(X, Y),
		Dimension(Width, Height)
	{}

public:
	Point Position;
	Point Dimension;
};

#endif