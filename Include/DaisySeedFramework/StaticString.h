#pragma once
#ifndef STATIC_STRING_H
#define STATIC_STRING_H

#include "StringUtils.h"
#include <DigitalSignalProcessing/DataTypes.h>

template<uint16_t MaxSize, uint16_t Capacity = MaxSize>
struct StaticString
{
public:
	StaticString(cstr Value = nullptr)
		: m_Buffer{}
	{
		Set(Value);
	}

	template<uint16_t OtherMaxSize, uint16_t OtherCapacity>
	StaticString(const StaticString<OtherMaxSize, OtherCapacity>& Value)
	{
		Set(Value.GetValue());
	}

	void Clear(void)
	{
		m_Buffer[0] = '\0';
	}

	bool Set(char Value, bool CutOverflow = false)
	{
		return Set(&Value, 1, CutOverflow);
	}

	bool Set(cstr Value, bool CutOverflow = false)
	{
		return SetString(Value, m_Buffer, Capacity, CutOverflow);
	}

	bool Set(cstr Value, uint16_t Length, bool CutOverflow = false)
	{
		return SetString(Value, Length, m_Buffer, Capacity, CutOverflow);
	}

	bool Append(char Value, bool CutOverflow = false)
	{
		return Append(&Value, 1, CutOverflow);
	}

	bool Append(cstr Value, bool CutOverflow = false)
	{
		return Append(Value, GetStringLength(Value), CutOverflow);
	}

	bool Append(cstr Value, uint16_t Length, bool CutOverflow = false)
	{
		const uint8_t ActualCapacity = Capacity - 1;

		uint16_t currentLen = GetStringLength(m_Buffer);
		uint16_t newLen = Length + currentLen;

		if (!CutOverflow && newLen > ActualCapacity)
			return false;

		uint16_t remaningLen = ActualCapacity - newLen;

		Memory::Copy(Value, m_Buffer + currentLen, Math::Min(remaningLen, newLen));

		m_Buffer[newLen] = '\0';

		return true;
	}

	str GetValue(void)
	{
		return m_Buffer;
	}

	cstr GetValue(void) const
	{
		return m_Buffer;
	}

	uint16_t GetLength(void) const
	{
		return GetStringLength(m_Buffer);
	}

	StaticString& operator+(char Value)
	{
		Append(Value);

		return *this;
	}

	StaticString& operator+(cstr Value)
	{
		Append(Value);

		return *this;
	}

	template<uint16_t OtherMaxSize, uint16_t OtherCapacity>
	StaticString& operator+(const StaticString<OtherMaxSize, OtherCapacity>& Value)
	{
		Append(Value.GetValue());

		return *this;
	}

	StaticString& operator=(char Value)
	{
		Set(Value);

		return *this;
	}

	StaticString& operator=(cstr Value)
	{
		Set(Value);

		return *this;
	}

	template<uint16_t OtherMaxSize, uint16_t OtherCapacity>
	StaticString& operator=(const StaticString<OtherMaxSize, OtherCapacity>& Value)
	{
		Set(Value.GetValue());

		return *this;
	}

	operator str(void)
	{
		return m_Buffer;
	}

	operator cstr(void) const
	{
		return m_Buffer;
	}

private:
	char m_Buffer[MaxSize];
};

#endif