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

	Rect(int16 X, int16 Y, int16 Width, int16 Height)
		: Position(X, Y),
		Dimension(Width, Height)
	{}

public:
	Point Position;
	Point Dimension;
};

#endif