#pragma once
#ifndef STATIC_STRING_H
#define STATIC_STRING_H

#include "StringUtils.h"
#include <DigitalSignalProcessing/DataTypes.h>

template<uint8_t MaxSize, uint8_t Capacity = MaxSize>
struct StaticString
{
public:
	StaticString(cstr Value = nullptr)
		: m_Buffer{}
	{
		Set(Value);
	}

	template<uint8_t OtherMaxSize, uint8_t OtherCapacity>
	StaticString(const StaticString<OtherMaxSize, OtherCapacity>& Value)
	{
		Set(Value.GetValue());
	}

	void Clear(void)
	{
		m_Buffer[0] = '\0';
	}

	bool Set(cstr Value, bool CutOverflow = false)
	{
		return SetString(Value, m_Buffer, Capacity, CutOverflow);
	}

	str GetValue(void)
	{
		return m_Buffer;
	}

	cstr GetValue(void) const
	{
		return m_Buffer;
	}

	StaticString& operator=(cstr Value)
	{
		Set(Value);

		return *this;
	}

	template<uint8_t OtherMaxSize, uint8_t OtherCapacity>
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