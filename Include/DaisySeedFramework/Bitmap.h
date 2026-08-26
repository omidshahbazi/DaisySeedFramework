#pragma once
#ifndef BITMAP_H
#define BITMAP_H

#include <DigitalSignalProcessing/DataTypes.h>

struct Bitmap
{
public:
	typedef uint64 DataType;

public:
	uint8 Width;
	uint8 Height;
	const DataType* const Data;
	uint8 BitsPerPixel;
};

#endif