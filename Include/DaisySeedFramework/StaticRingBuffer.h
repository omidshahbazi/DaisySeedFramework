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
	RingBuffer(T* Buffer = nullptr, uint16 Capacity = 0)
		: m_Buffer(Buffer),
		m_Capacity(Capacity),
		m_Head(0),
		m_Tail(0),
		m_Count(0)
	{}

	bool Push(const T& Value)
	{
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

	uint16 Push(const T* Values, uint16 Count)
	{
		uint16 written = 0;
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
		if (m_Count == 0)
			return false;

		OutValue = m_Buffer[m_Tail];
		m_Tail = (m_Tail + 1) % m_Capacity;
		--m_Count;
		return true;
	}

	uint16 Pop(T* OutBuffer, uint16 Count)
	{
		uint16 readCount = 0;
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

	uint16 GetSize(void) const
	{
		return m_Count;
	}

	uint16 GetCapacity(void) const
	{
		return m_Capacity;
	}

	uint16 GetFreeSpace(void) const
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

private:
	T* m_Buffer;
	uint16 m_Capacity;
	uint16 m_Head;
	uint16 m_Tail;
	uint16 m_Count;
};

template <typename T, uint16 MaxSize, bool AllowOverwrite = true>
struct StaticRingBuffer : public RingBuffer<T, AllowOverwrite>
{
public:
	StaticRingBuffer(void)
		: RingBuffer<T, AllowOverwrite>(m_Buffer, MaxSize)
	{}

private:
	T m_Buffer[MaxSize];
};

#endif