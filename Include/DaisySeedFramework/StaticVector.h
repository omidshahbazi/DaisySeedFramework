#pragma once
#ifndef STATIC_VECTOR_H
#define STATIC_VECTOR_H

#include <DigitalSignalProcessing/Debug.h>
#include <DigitalSignalProcessing/Memory.h>

template <typename T>
struct Vector
{
public:
	typedef T ItemType;

public:
	Vector(T* Buffer = nullptr, uint16_t Capacity = 0)
		: m_Buffer(Buffer),
		m_Capacity(Capacity),
		m_Count(0)
	{}

	void PushBack(const T& Value)
	{
		ASSERT(m_Count < m_Capacity, "Out of Size");

		m_Buffer[m_Count++] = Value;
	}

	void PushBack(const T* const Value, uint16_t Count)
	{
		for (uint16_t i = 0; i < Count; ++i)
			PushBack(Value[i]);
	}

	void Insert(uint16_t Index, const T& Value)
	{
		ASSERT(m_Count < m_Capacity, "Out of Size");
		ASSERT(Index <= m_Count, "Index Out of Range");

		for (uint16_t i = m_Count; i > Index; --i)
			m_Buffer[i] = m_Buffer[i - 1];

		m_Buffer[Index] = Value;
		++m_Count;
	}

	void PopBack()
	{
		ASSERT(0 < m_Count, "Out of Size");

		--m_Count;
		m_Buffer[m_Count] = {};
	}

	T& Front(void)
	{
		ASSERT(0 < m_Count, "Out of Size");
		return m_Buffer[0];
	}

	const T& Front(void) const
	{
		ASSERT(0 < m_Count, "Out of Size");
		return m_Buffer[0];
	}

	T& Back(void)
	{
		ASSERT(0 < m_Count, "Out of Size");
		return m_Buffer[m_Count - 1];
	}

	const T& Back(void) const
	{
		ASSERT(0 < m_Count, "Out of Size");
		return m_Buffer[m_Count - 1];
	}

	void RemoveAt(uint16_t Index)
	{
		ASSERT(Index < m_Count, "Index Out of Range");

		for (uint16_t i = Index; i < m_Count - 1; ++i)
			m_Buffer[i] = m_Buffer[i + 1];

		--m_Count;
		m_Buffer[m_Count] = {};
	}

	void RemoveRange(uint16_t Index, uint16_t Count)
	{
		if (Count == 0)
			return;

		ASSERT(Index < m_Count, "Index Out of Range");
		ASSERT(Index + Count <= m_Count, "Range Out of Range");

		uint16_t elementsAfterRange = m_Count - (Index + Count);

		if (elementsAfterRange > 0)
			Memory::Copy(m_Buffer + Index + Count, m_Buffer + Index, elementsAfterRange);

		m_Count -= Count;
	}

	void Clear(void)
	{
		for (uint16_t i = 0; i < m_Count; ++i)
		{
			m_Buffer[i].~T();
			m_Buffer[i] = {};
		}

		m_Count = 0;
	}

	T* GetData(void)
	{
		return m_Buffer;
	}

	const T* GetData(void) const
	{
		return m_Buffer;
	}

	uint16_t GetSize(void) const
	{
		return m_Count;
	}

	uint16_t GetCapacity(void) const
	{
		return m_Capacity;
	}

	bool HasFree(void) const
	{
		return m_Count < m_Capacity;
	}

	bool IsEmpty(void) const
	{
		return m_Count == 0;
	}

	T& operator[](uint16_t Index)
	{
		ASSERT(Index < m_Count, "Index Out of Range");
		return m_Buffer[Index];
	}

	const T& operator[](uint16_t Index) const
	{
		ASSERT(Index < m_Count, "Index Out of Range");
		return m_Buffer[Index];
	}

private:
	T* m_Buffer;
	uint16_t m_Capacity;
	uint16_t m_Count;
};

template <typename T, uint16_t MaxSize>
struct StaticVector : public Vector<T>
{
public:
	StaticVector(void) : Vector<T>(m_Buffer, MaxSize)
	{}

private:
	T m_Buffer[MaxSize];
};

#endif