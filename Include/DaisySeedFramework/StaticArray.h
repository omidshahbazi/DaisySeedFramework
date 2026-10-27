#pragma once
#ifndef STATIC_ARRAY_H
#define STATIC_ARRAY_H

#include <DigitalSignalProcessing/Debug.h>

template<typename T, uint16_t Length>
struct StaticArray
{
	static_assert(Length != 0, "Length must be greater than zero");

public:
	StaticArray(void)
		: m_Buffer{}
	{}

	void Clear(void)
	{
		for (uint16_t i = 0; i < Length; ++i)
		{
			m_Buffer[i].~T();

			if constexpr (is_trivially_copyable_v<T>)
				m_Buffer[i] = {};
		}
	}

	int16_t IndexOf(const T& Value, uint16_t StartIndex = 0) const
	{
		for (uint16_t i = StartIndex; i < Length; ++i)
			if (m_Buffer[i] == Value)
				return i;

		return -1;
	}

	template <typename Predicate_t>
	int16_t IndexOf(Predicate_t Predicate, uint16_t StartIndex = 0) const
	{
		for (uint16_t i = StartIndex; i < Length; ++i)
			if (Predicate(m_Buffer[i]))
				return i;

		return -1;
	}

	bool Contains(const T& Value) const
	{
		return (IndexOf(Value) != -1);
	}

	template <typename Predicate_t>
	bool Contains(Predicate_t Predicate) const
	{
		return (IndexOf(Predicate) != -1);
	}

	T* GetData(void)
	{
		return m_Buffer;
	}

	const T* GetData(void) const
	{
		return m_Buffer;
	}

	explicit operator T* (void) const
	{
		return GetData();
	}

	constexpr uint16_t GetLength(void) const
	{
		return Length;
	}

	StaticArray& operator=(const StaticArray& Other)
	{
		for (uint16_t i = 0; i < Length; ++i)
			m_Buffer[i] = Other.m_Buffer[i];

		return *this;
	}

	T& operator[](uint16_t Index)
	{
		ASSERT(Index < Length, "Index [%u] must be less than Length [%u]", Index, Length);

		return m_Buffer[Index];
	}

	const T& operator[](uint16_t Index) const
	{
		ASSERT(Index < Length, "Index [%u] must be less than Length [%u]", Index, Length);

		return m_Buffer[Index];
	}

private:
	T m_Buffer[Length];
};

#endif