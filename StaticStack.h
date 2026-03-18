#pragma once
#ifndef STATIC_STACK_H
#define STATIC_STACK_H

#include "DSP/Debug.h"

template <typename T, uint8 MaxSize>
struct StaticStack
{
public:
	typedef T ItemType;

public:
	StaticStack(void)
		: m_Buffer{},
		  m_Count(0)
	{
	}

	void Push(const T &Value)
	{
		ASSERT(m_Count < MaxSize, "Out of Size");

		m_Buffer[m_Count++] = Value;
	}

	void Pop()
	{
		T temp;
		Pop(temp);
	}

	void Pop(T &Value)
	{
		ASSERT(0 < m_Count, "Out of Size");

		--m_Count;

		Value = m_Buffer[m_Count];
		m_Buffer[m_Count] = {};
	}

	T &Back(void)
	{
		ASSERT(0 < m_Count, "Out of Size");

		return m_Buffer[0];
	}

	const T &Back(void) const
	{
		ASSERT(0 < m_Count, "Out of Size");

		return m_Buffer[0];
	}

	T &Front(void)
	{
		ASSERT(0 < m_Count, "Out of Size");

		return m_Buffer[m_Count - 1];
	}

	const T &Front(void) const
	{
		ASSERT(0 < m_Count, "Out of Size");

		return m_Buffer[m_Count - 1];
	}

	void Clear(void)
	{
		for (uint8 i = 0; i < m_Count; ++i)
		{
			m_Buffer[i].~T();
			m_Buffer[i] = {};
		}

		m_Count = 0;
	}

	uint8 GetSize(void) const
	{
		return m_Count;
	}

	bool HasFree(void) const
	{
		return m_Count < MaxSize;
	}

	bool IsEmpty(void) const
	{
		return m_Count == 0;
	}

	T &operator[](uint8 Index)
	{
		ASSERT(Index < MaxSize, "Out of Size");

		return m_Buffer[Index];
	}

	const T &operator[](uint8 Index) const
	{
		ASSERT(Index < MaxSize, "Out of Size");

		return m_Buffer[Index];
	}

private:
	T m_Buffer[MaxSize];
	uint8 m_Count;
};

#endif