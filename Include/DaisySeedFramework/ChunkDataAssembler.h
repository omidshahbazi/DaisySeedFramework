#pragma once
#ifndef CHUNCK_DATA_ASSEMBLER
#define CHUNCK_DATA_ASSEMBLER

#include "Common.h"
#include "StaticVector.h"

template<uint16_t BufferSize>
class ChunkDataAssembler
{
private:
	typedef StaticVector<uint8_t, BufferSize> BufferType;

protected:
	ChunkDataAssembler(void)
		: m_ExpectedBytesSize(0)
	{}

	void Transmit(const uint8_t* Buffer, uint16_t Length)
	{
		HandleRawTransmit(reinterpret_cast<const uint8_t*>(&Length), sizeof(Length));
		HandleRawTransmit(Buffer, Length);
	}

	virtual void HandlePacket(const uint8_t* Buffer, uint16_t Length) = 0;

	virtual void HandleRawTransmit(const uint8_t* Buffer, uint16_t Length) = 0;

	void HandleRawPacket(const uint8_t* Buffer, uint8_t Length)
	{
		m_Buffer.PushBack(Buffer, Length);

		const uint8_t HeaderSize = sizeof(m_ExpectedBytesSize);

		while (m_Buffer.GetSize() >= HeaderSize)
		{
			const uint8_t* buffer = m_Buffer.GetData();

			ReadAndAdvanceBuffer(buffer, m_ExpectedBytesSize);

			const uint16_t ExpectedBufferSize = HeaderSize + m_ExpectedBytesSize;

			if (m_Buffer.GetSize() > ExpectedBufferSize)
			{
				ResetAssembler();
				break;
			}

			if (m_Buffer.GetSize() < ExpectedBufferSize)
				break;

			HandlePacket(buffer, m_ExpectedBytesSize);

			m_Buffer.RemoveRange(0, ExpectedBufferSize);
		}
	}

	//void TransmitFragmented(const uint8_t* Buffer, uint16_t Length, uint16_t Delay) const
	//{
	//	uint16_t index = 0;
	//	while (index < Length)
	//	{
	//		const uint16_t CountPerStep = 64;

	//		uint16_t countPerStep = (uint16_t)Math::Min(CountPerStep, Length - index);

	//		HandleRawTransmit(const_cast<uint8_t*>(Buffer + index), countPerStep);

	//		index += CountPerStep;

	//		//delay?
	//	}
	//}

private:
	void ResetAssembler(void)
	{
		m_Buffer.Clear();
		m_ExpectedBytesSize = 0;
	}

protected:
	template <typename T>
	static uint16_t ReadAndAdvanceBuffer(const uint8_t*& Buffer, T& Data)
	{
		Data = *(T*)Buffer;
		Buffer += sizeof(T);

		return sizeof(T);
	}

private:
	BufferType m_Buffer;
	uint16_t m_ExpectedBytesSize;
};

#endif