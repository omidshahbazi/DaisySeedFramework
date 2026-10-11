#pragma once
#ifndef STATIC_STRING_H
#define STATIC_STRING_H

#include "StringUtils.h"
#include <DigitalSignalProcessing/DataTypes.h>
#include <DigitalSignalProcessing/Memory.h>

template<typename CharType, uint16_t MaxCapacity, uint16_t Capacity = MaxCapacity>
struct BasicStaticString
{
	static_assert(Capacity <= MaxCapacity, "Capacity cannot be more than MaxCapacity");

public:
	static constexpr uint16_t MaxCapacity = MaxCapacity;
	static constexpr uint16_t Capacity = Capacity;

public:
	typedef CharType* CharPointer;
	typedef const CharType* ConstCharPointer;

public:
	BasicStaticString(ConstCharPointer Value = nullptr)
		: m_Buffer{}
	{
		Set(Value);
	}

	template<uint16_t OtherMaxCapacity, uint16_t OtherCapacity>
	BasicStaticString(const BasicStaticString<CharType, OtherMaxCapacity, OtherCapacity>& Value)
	{
		static_assert(OtherCapacity <= Capacity, "OtherCapacity must be less than or equal to Capacity");

		Set(Value.GetValue());
	}

	template<typename OtherCharType>
	BasicStaticString(const OtherCharType* Begin, const OtherCharType* End)
		: m_Buffer{}
	{
		ASSERT(Begin != nullptr, "Begin cannot be null");
		ASSERT(End != nullptr, "Begin cannot be null");
		ASSERT(Begin <= End, "Begin must be before the End");

		const uint16_t ActualCapacity = Capacity - 1;

		uint16_t i = 0;
		while (Begin + i < End && i < ActualCapacity)
		{
			m_Buffer[i] = (CharType)Begin[i];
			i++;
		}

		m_Buffer[i] = '\0';
	}

	void Clear(void)
	{
		m_Buffer[0] = '\0';
	}

	bool Set(CharType Value, bool CutOverflow = false)
	{
		return Set(&Value, 1, CutOverflow);
	}

	bool Set(ConstCharPointer Value, bool CutOverflow = false)
	{
		return SetString(Value, m_Buffer, Capacity, CutOverflow);
	}

	bool Set(ConstCharPointer Value, uint16_t Length, bool CutOverflow = false)
	{
		return SetString(Value, Length, m_Buffer, Capacity, CutOverflow);
	}

	bool Append(CharType Value, bool CutOverflow = false)
	{
		return Append(&Value, 1, CutOverflow);
	}

	bool Append(ConstCharPointer Value, bool CutOverflow = false)
	{
		return Append(Value, GetStringLength(Value), CutOverflow);
	}

	bool Append(ConstCharPointer Value, uint16_t Length, bool CutOverflow = false)
	{
		const uint8_t ActualCapacity = Capacity - 1;

		uint16_t currentLen = GetStringLength(m_Buffer);
		uint16_t newLen = Length + currentLen;

		if (!CutOverflow && newLen > ActualCapacity)
			return false;

		uint16_t remainingLen = ActualCapacity - newLen;

		Memory::Copy(Value, m_Buffer + currentLen, Math::Min(remainingLen, newLen));

		m_Buffer[newLen] = '\0';

		return true;
	}

	CharPointer GetValue(void)
	{
		return m_Buffer;
	}

	ConstCharPointer GetValue(void) const
	{
		return m_Buffer;
	}

	uint16_t GetLength(void) const
	{
		return GetStringLength(m_Buffer);
	}

	BasicStaticString& operator+(CharType Value)
	{
		Append(Value);

		return *this;
	}

	BasicStaticString& operator+(ConstCharPointer Value)
	{
		Append(Value);

		return *this;
	}

	template<uint16_t OtherMaxCapacity, uint16_t OtherCapacity>
	BasicStaticString& operator+(const BasicStaticString<CharType, OtherMaxCapacity, OtherCapacity>& Value)
	{
		Append(Value.GetValue());

		return *this;
	}

	BasicStaticString& operator=(CharType Value)
	{
		Set(Value);

		return *this;
	}

	BasicStaticString& operator=(ConstCharPointer Value)
	{
		Set(Value);

		return *this;
	}

	template<uint16_t OtherMaxCapacity, uint16_t OtherCapacity>
	BasicStaticString& operator=(const BasicStaticString<CharType, OtherMaxCapacity, OtherCapacity>& Value)
	{
		Set(Value.GetValue());

		return *this;
	}

	BasicStaticString& operator+=(CharType Value)
	{
		Append(Value);

		return *this;
	}

	BasicStaticString& operator+=(ConstCharPointer Value)
	{
		Append(Value);

		return *this;
	}

	template<uint16_t OtherMaxCapacity, uint16_t OtherCapacity>
	BasicStaticString& operator+=(const BasicStaticString<CharType, OtherMaxCapacity, OtherCapacity>& Value)
	{
		Append(Value.GetValue());

		return *this;
	}

	operator CharPointer(void)
	{
		return m_Buffer;
	}

	operator ConstCharPointer(void) const
	{
		return m_Buffer;
	}

private:
	CharType m_Buffer[MaxCapacity];
};

template<uint16_t MaxCapacity, uint16_t Capacity = MaxCapacity>
using StaticString = BasicStaticString<char, MaxCapacity, Capacity>;

template<uint16_t MaxCapacity, uint16_t Capacity = MaxCapacity>
using StaticWString = BasicStaticString<wchar_t, MaxCapacity, Capacity>;

#endif