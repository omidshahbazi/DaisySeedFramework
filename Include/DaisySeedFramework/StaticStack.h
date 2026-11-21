#pragma once
#ifndef STATIC_STACK_H
#define STATIC_STACK_H

#include <DigitalSignalProcessing/Debug.h>

template <typename T>
struct Stack
{
public:
	typedef T ItemType;

public:
	Stack(NotNull<T> Buffer, puint16_t Capacity)
		: m_Buffer(Buffer),
		m_Capacity(Capacity),
		m_Count(0)
	{}

	Stack(const Stack&) = delete;

	void Push(const T& Value)
	{
		ASSERT_VALUE_LESS(m_Count, m_Capacity);

		m_Buffer[m_Count++] = Value;
	}

	void Pop()
	{
		T temp;
		Pop(temp);
	}

	void Pop(T& Value)
	{
		ASSERT_VALUE_GREATER(m_Count, 0);

		--m_Count;

		Value = m_Buffer[m_Count];
		m_Buffer[m_Count] = {};
	}

	T& Back(void)
	{
		ASSERT_VALUE_GREATER(m_Count, 0);

		return m_Buffer[0];
	}

	const T& Back(void) const
	{
		ASSERT_VALUE_GREATER(m_Count, 0);

		return m_Buffer[0];
	}

	T& Front(void)
	{
		ASSERT_VALUE_GREATER(m_Count, 0);

		return m_Buffer[m_Count - 1];
	}

	const T& Front(void) const
	{
		ASSERT_VALUE_GREATER(m_Count, 0);

		return m_Buffer[m_Count - 1];
	}

	void Clear(void)
	{
		for (uint16_t i = 0; i < m_Count; ++i)
		{
			m_Buffer[i].~T();

			if constexpr (is_trivially_copyable_v<T>)
				m_Buffer[i] = {};
		}

		m_Count = 0;
	}

	int16_t IndexOf(const T& Value, uint16_t StartIndex = 0) const
	{
		for (uint16_t i = StartIndex; i < m_Count; ++i)
			if (m_Buffer[i] == Value)
				return i;

		return -1;
	}

	template <typename Predicate_t>
	int16_t IndexOf(Predicate_t Predicate, uint16_t StartIndex = 0) const
	{
		for (uint16_t i = StartIndex; i < m_Count; ++i)
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

	uint16_t GetSize(void) const
	{
		return m_Count;
	}

	bool HasFree(void) const
	{
		return m_Count < m_Capacity;
	}

	bool IsEmpty(void) const
	{
		return m_Count == 0;
	}

	Stack& operator=(const Stack& Other)
	{
		Clear();

		Push(Other.m_Buffer, Other.m_Count);

		return *this;
	}

	T& operator[](uint16_t Index)
	{
		ASSERT_VALUE_LESS(Index, m_Count);

		return m_Buffer[Index];
	}

	const T& operator[](uint16_t Index) const
	{
		ASSERT_VALUE_LESS(Index, m_Count);

		return m_Buffer[Index];
	}

private:
	T* m_Buffer;
	uint16_t m_Capacity;
	uint16_t m_Count;
};

template <typename T, uint16_t MaxSize>
struct StaticStack : public Stack<T>
{
	STATIC_ASSERT(MaxSize != 0, "MaxSize must be greater than zero");

public:
	StaticStack(void) : Stack<T>(m_Buffer, MaxSize)
	{}

	template<uint16_t OtherMaxSize>
	StaticStack(const StaticStack<T, OtherMaxSize>& Other)
	{
		STATIC_ASSERT(OtherMaxSize <= MaxSize, "OtherMaxSize must be less than or equal to MaxSize");

		this->Clear();

		this->Push(Other.m_Buffer, Other.GetSize());
	}

private:
	T m_Buffer[MaxSize];
};

#endif