#pragma once
#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <DigitalSignalProcessing/DataTypes.h>

uint16_t GetStringLength(cstr Value);

cstr IntToString(int32_t Value);

cstr FloatToString(float Value, uint8_t Decimals = 1);

template <typename T>
static cstr ToString(T Value)
{
	return IntToString(Value);
}

template <>
cstr ToString<float>(float Value)
{
	return FloatToString(Value);
}

uint8_t GetNoteIndex(uint8_t MIDINumber);

cstr GetEnglishNoteName(uint8_t MIDINumber);

cstr GetFrenchNoteName(uint8_t MIDINumber);

bool SetString(cstr Value, str Buffer, uint8_t BufferLength, bool CutOverflow = false);

#endif