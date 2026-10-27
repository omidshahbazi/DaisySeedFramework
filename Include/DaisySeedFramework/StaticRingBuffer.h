#pragma once
#ifndef STATIC_RING_BUFFER_H
#define STATIC_RING_BUFFER_H

#include <DigitalSignalProcessing/Debug.h>
#include <DigitalSignalProcessing/Memory.h>

template <typename T, bool AllowOverwrite = true>
struct RingBuffer
{
public:
	typedef T ItemType;

public:
	RingBuffer(T* Buffer = nullptr, uint16_t Capacity = 0)
		: m_Buffer(Buffer),
		m_Capacity(Capacity),
		m_Head(0),
		m_Tail(0),
		m_Count(0)
	{
		ASSERT(Buffer != nullptr, "Buffer cannot be null");
		ASSERT(Capacity != 0, "Capacity must be greater than zero");
	}

	RingBuffer(const RingBuffer&) = delete;

	bool Push(const T& Value)
	{
		ASSERT(m_Buffer != nullptr, "m_Buffer cannot be null");

		if (m_Count >= m_Capacity)
		{
			if constexpr (AllowOverwrite)
			{
				m_Buffer[m_Head] = Value;
				m_Head = (m_Head + 1) % m_Capacity;
				m_Tail = (m_Tail + 1) % m_Capacity;
				return true;
			}
			else
			{
				return false;
			}
		}

		m_Buffer[m_Head] = Value;
		m_Head = (m_Head + 1) % m_Capacity;
		++m_Count;
		return true;
	}

	uint16_t Push(const T* Values, uint16_t Count)
	{
		ASSERT(m_Buffer != nullptr, "m_Buffer cannot be null");

		uint16_t written = 0;
		while (written < Count)
		{
			if (m_Count >= m_Capacity)
			{
				if constexpr (AllowOverwrite)
				{
					m_Buffer[m_Head] = Values[written];
					m_Head = (m_Head + 1) % m_Capacity;
					m_Tail = (m_Tail + 1) % m_Capacity;
				}
				else
				{
					break;
				}
			}
			else
			{
				m_Buffer[m_Head] = Values[written];
				m_Head = (m_Head + 1) % m_Capacity;
				++m_Count;
			}
			++written;
		}
		return written;
	}

	bool Pop(T& OutValue)
	{
		ASSERT(m_Buffer != nullptr, "m_Buffer cannot be null");

		if (m_Count == 0)
			return false;

		OutValue = m_Buffer[m_Tail];
		m_Tail = (m_Tail + 1) % m_Capacity;
		--m_Count;
		return true;
	}

	uint16_t Pop(T* OutBuffer, uint16_t Count)
	{
		ASSERT(m_Buffer != nullptr, "m_Buffer cannot be null");
		ASSERT(OutBuffer != nullptr, "OutBuffer cannot be null");

		uint16_t readCount = 0;
		while (readCount < Count && m_Count > 0)
		{
			if (OutBuffer != nullptr)
				OutBuffer[readCount] = m_Buffer[m_Tail];

			m_Tail = (m_Tail + 1) % m_Capacity;
			--m_Count;
			++readCount;
		}
		return readCount;
	}

	void Clear(void)
	{
		m_Head = 0;
		m_Tail = 0;
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

	uint16_t GetFreeSpace(void) const
	{
		return m_Capacity - m_Count;
	}

	bool IsEmpty(void) const
	{
		return m_Count == 0;
	}

	bool IsFull(void) const
	{
		return m_Count == m_Capacity;
	}

	RingBuffer& operator=(const RingBuffer& Other)
	{
		Clear();

		Push(Other.m_Buffer, Other.m_Count);

		return *this;
	}

	T& operator[](uint16_t Index)
	{
		ASSERT(m_Buffer != nullptr, "m_Buffer cannot be null");
		ASSERT(Index < m_Capacity, "Out of Size");

		return m_Buffer[Index];
	}

	const T& operator[](uint16_t Index) const
	{
		ASSERT(m_Buffer != nullptr, "m_Buffer cannot be null");
		ASSERT(Index < m_Capacity, "Out of Size");

		return m_Buffer[Index];
	}

private:
	T* m_Buffer;
	uint16_t m_Capacity;
	uint16_t m_Head;
	uint16_t m_Tail;
	uint16_t m_Count;
};

template <typename T, uint16_t MaxSize, bool AllowOverwrite = true>
struct StaticRingBuffer : public RingBuffer<T, AllowOverwrite>
{
	static_assert(MaxSize != 0, "MaxSize must be greater than zero");

public:
	StaticRingBuffer(void)
		: RingBuffer<T, AllowOverwrite>(m_Buffer, MaxSize)
	{}

	template<uint16_t OtherMaxSize>
	StaticRingBuffer(const StaticRingBuffer<T, OtherMaxSize>&Other)
	{
		static_assert(OtherMaxSize <= MaxSize, "OtherMaxSize must be less than or equal to MaxSize");

		this->Clear();

		this->Push(Other.m_Buffer, Other.GetSize());
	}

private:
	T m_Buffer[MaxSize];
};

#endif