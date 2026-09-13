#pragma once
#ifndef STATIC_STRING_H
#define STATIC_STRING_H

#include "StringUtils.h"

template <uint8_t MaxSize, uint8_t Capacity = MaxSize>
struct StaticString
{
public:
	StaticString(cstr Value = nullptr)
		: m_Buffer{}
	{
		Set(Value);
	}

	void Clear(void)
	{
		m_Buffer[0] = '\0';
	}

	bool Set(cstr Value, bool CutOverflow = false)
	{
		Clear();

		return SetString(Value, m_Buffer, Capacity, CutOverflow);
	}

	StaticString& operator=(cstr Value)
	{
		Clear();

		SetString(Value, m_Buffer, Capacity, false);

		return *this;
	}

	cstr GetValue(void) const
	{
		return m_Buffer;
	}

private:
	char m_Buffer[MaxSize];
};

#endif