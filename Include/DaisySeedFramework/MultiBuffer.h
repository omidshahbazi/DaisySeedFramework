#pragma once
#ifndef MULTI_BUFFER_H
#define MULTI_BUFFER_H

#include <DigitalSignalProcessing/Math.h>

template<typename T, uint8_t BufferCount>
struct MultiBuffer
{
public:
	MultiBuffer(void)
		: m_Buffers{},
		m_FrontBufferIndex(0)
	{}

	void SwapBuffers(void)
	{
		m_FrontBufferIndex = Math::Wrap(m_FrontBufferIndex + 1, 0, BufferCount - 1);
	}

	T& GetFrontBuffer(void)
	{
		return m_Buffers[m_FrontBufferIndex];
	}
	const T& GetFrontBuffer(void) const
	{
		return m_Buffers[m_FrontBufferIndex];
	}

	T& GetBackBuffer(void)
	{
		uint8_t index = Math::Wrap(m_FrontBufferIndex - 1, 0, BufferCount - 1);

		return m_Buffers[index];
	}
	const T& GetBackBuffer(void) const
	{
		uint8_t index = Math::Wrap(m_FrontBufferIndex - 1, 0, BufferCount - 1);

		return m_Buffers[index];
	}

private:
	T m_Buffers[BufferCount];
	uint8_t m_FrontBufferIndex;
};

template<typename T>
using DoubleBuffer = MultiBuffer<T, 2>;

template<typename T>
using TripleBuffer = MultiBuffer<T, 2>;

#endif