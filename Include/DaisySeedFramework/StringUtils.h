#pragma once
#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <DigitalSignalProcessing/DataTypes.h>
#include <DigitalSignalProcessing/Memory.h>

template<typename CharType>
uint16_t GetStringLength(const CharType* Value)
{
	if (Value == nullptr)
		return 0;

	uint16_t len = 0;
	while (Value[len] != 0)
		++len;

	return len;
}

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

template<typename CharType>
bool SetString(const CharType* Value, uint16_t Length, CharType* Buffer, uint8_t BufferLength, bool CutOverflow = false)
{
	const uint8_t ActualBufferSize = BufferLength - 1;

	if (Length > ActualBufferSize)
		if (CutOverflow)
			Length = ActualBufferSize;
		else
			return false;

	if (Length != 0)
		Memory::Copy(Value, Buffer, Length);

	Buffer[Length] = 0;

	return true;
}

template<typename CharType>
bool SetString(const CharType* Value, CharType* Buffer, uint8_t BufferLength, bool CutOverflow = false)
{
	uint8_t length = GetStringLength(Value);

	return SetString(Value, length, Buffer, BufferLength, CutOverflow);
}

#endif