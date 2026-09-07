#pragma once
#ifndef BITMAP_H
#define BITMAP_H

#include <DigitalSignalProcessing/DataTypes.h>

struct Bitmap
{
public:
	typedef uint64_t DataType;

public:
	uint8_t Width;
	uint8_t Height;
	const DataType* const Data;
	uint8_t BitsPerPixel;
};

#endif