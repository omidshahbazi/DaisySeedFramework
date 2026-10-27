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
	{
		ASSERT(Buffer != nullptr, "Buffer cannot be null");
		ASSERT(Capacity != 0, "Capacity must be greater than zero");
	}

	Vector(const Vector&) = delete;

	void Extend(uint16_t Count)
	{
		ASSERT(m_Count + Count <= m_Capacity, "Out of Size");

		m_Count += Count;
	}

	T& Push(const T& Value)
	{
		ASSERT(m_Buffer != nullptr, "m_Buffer cannot be null");
		ASSERT(m_Count < m_Capacity, "Out of Size");

		m_Buffer[m_Count++] = Value;

		return m_Buffer[m_Count - 1];
	}

	void Push(const T* const Value, uint16_t Count)
	{
		for (uint16_t i = 0; i < Count; ++i)
			Push(Value[i]);
	}

	void Push(const Vector<T>& Value)
	{
		Push(Value.GetData(), Value.GetSize());
	}

	T& Insert(uint16_t Index, const T& Value)
	{
		ASSERT(m_Buffer != nullptr, "m_Buffer cannot be null");
		ASSERT(m_Count < m_Capacity, "Out of Size");
		ASSERT(Index <= m_Count, "Index Out of Range");

		for (uint16_t i = m_Count; i > Index; --i)
			m_Buffer[i] = m_Buffer[i - 1];

		m_Buffer[Index] = Value;
		++m_Count;

		return m_Buffer[Index];
	}

	void Insert(uint16_t Index, const T* const Value, uint16_t Count)
	{
		ASSERT(m_Buffer != nullptr, "m_Buffer cannot be null");
		ASSERT(m_Count + Count <= m_Capacity, "Out of Size");
		ASSERT(Index <= m_Count, "Index Out of Range");

		for (int32_t i = static_cast<int32_t>(m_Count) - 1; i >= Index; --i)
			m_Buffer[i + Count] = m_Buffer[i];

		for (uint16_t i = 0; i < Count; ++i)
			m_Buffer[Index + i] = Value[i];

		m_Count += Count;
	}

	void Insert(uint16_t Index, const Vector<T>& Value)
	{
		Insert(Index, Value.GetData(), Value.GetSize());
	}

	void Pop(void)
	{
		ASSERT(m_Buffer != nullptr, "m_Buffer cannot be null");
		ASSERT(0 < m_Count, "Out of Size");

		--m_Count;
		m_Buffer[m_Count] = {};
	}

	T& Front(void)
	{
		ASSERT(m_Buffer != nullptr, "m_Buffer cannot be null");
		ASSERT(0 < m_Count, "Out of Size");

		return m_Buffer[0];
	}

	const T& Front(void) const
	{
		ASSERT(m_Buffer != nullptr, "m_Buffer cannot be null");
		ASSERT(0 < m_Count, "Out of Size");

		return m_Buffer[0];
	}

	T& Back(void)
	{
		ASSERT(m_Buffer != nullptr, "m_Buffer cannot be null");
		ASSERT(0 < m_Count, "Out of Size");

		return m_Buffer[m_Count - 1];
	}

	const T& Back(void) const
	{
		ASSERT(m_Buffer != nullptr, "m_Buffer cannot be null");
		ASSERT(0 < m_Count, "Out of Size");

		return m_Buffer[m_Count - 1];
	}

	void RemoveAt(uint16_t Index)
	{
		ASSERT(m_Buffer != nullptr, "m_Buffer cannot be null");
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

		ASSERT(m_Buffer != nullptr, "m_Buffer cannot be null");
		ASSERT(Index < m_Count, "Index Out of Range");
		ASSERT(Index + Count <= m_Count, "Range Out of Range");

		uint16_t elementsAfterRange = m_Count - (Index + Count);

		if (elementsAfterRange > 0)
			Memory::Copy(m_Buffer + Index + Count, m_Buffer + Index, elementsAfterRange);

		m_Count -= Count;
	}

	void Clear(void)
	{
		ASSERT(m_Buffer != nullptr, "m_Buffer cannot be null");

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
		ASSERT(m_Buffer != nullptr, "m_Buffer cannot be null");

		for (uint16_t i = StartIndex; i < m_Count; ++i)
			if (m_Buffer[i] == Value)
				return i;

		return -1;
	}

	template <typename Predicate_t>
	int16_t IndexOf(Predicate_t Predicate, uint16_t StartIndex = 0) const
	{
		ASSERT(m_Buffer != nullptr, "m_Buffer cannot be null");

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

	Vector& operator=(const Vector& Other)
	{
		Clear();

		Push(Other.m_Buffer, Other.m_Count);

		return *this;
	}

	T& operator[](uint16_t Index)
	{
		ASSERT(m_Buffer != nullptr, "m_Buffer cannot be null");
		ASSERT(Index < m_Count, "Index [%u] must be less than Count [%u]", Index, m_Count);

		return m_Buffer[Index];
	}

	const T& operator[](uint16_t Index) const
	{
		ASSERT(m_Buffer != nullptr, "m_Buffer cannot be null");
		ASSERT(Index < m_Count, "Index [%u] must be less than Count [%u]", Index, m_Count);

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
	static_assert(MaxSize != 0, "MaxSize must be greater than zero");

public:
	StaticVector(void) : Vector<T>(m_Buffer, MaxSize)
	{}

	template<uint16_t OtherMaxSize>
	StaticVector(const StaticVector<T, OtherMaxSize>& Other)
	{
		static_assert(OtherMaxSize <= MaxSize, "OtherMaxSize must be less than or equal to MaxSize");

		this->Clear();

		this->Push(Other.m_Buffer, Other.GetSize());
	}

private:
	T m_Buffer[MaxSize];
};

#endif