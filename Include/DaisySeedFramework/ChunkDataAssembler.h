#pragma once
#ifndef CHUNCK_DATA_ASSEMBLER
#define CHUNCK_DATA_ASSEMBLER

#include "Common.h"
#include "StaticVector.h"

template<uint16 BufferSize>
class ChunkDataAssembler
{
private:
	typedef StaticVector<uint8, BufferSize> BufferType;

protected:
	ChunkDataAssembler(void)
		: m_ExpectedBytesSize(0)
	{}

	void Transmit(const uint8* Buffer, uint16 Length)
	{
		HandleRawTransmit(reinterpret_cast<const uint8*>(&Length), sizeof(Length));
		HandleRawTransmit(Buffer, Length);
	}

	virtual void HandlePacket(const uint8* Buffer, uint16 Length) = 0;

	virtual void HandleRawTransmit(const uint8* Buffer, uint16 Length) = 0;

	void HandleRawPacket(const uint8* Buffer, uint8 Length)
	{
		m_Buffer.PushBack(Buffer, Length);

		const uint8 HeaderSize = sizeof(m_ExpectedBytesSize);

		while (m_Buffer.GetSize() >= HeaderSize)
		{
			const uint8* buffer = m_Buffer.GetData();

			ReadAndAdvanceBuffer(buffer, m_ExpectedBytesSize);

			const uint16 ExpectedBufferSize = HeaderSize + m_ExpectedBytesSize;

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

	//void TransmitFragmented(const uint8* Buffer, uint16 Length, uint16 Delay) const
	//{
	//	uint16 index = 0;
	//	while (index < Length)
	//	{
	//		const uint16 CountPerStep = 64;

	//		uint16 countPerStep = (uint16)Math::Min(CountPerStep, Length - index);

	//		HandleRawTransmit(const_cast<uint8*>(Buffer + index), countPerStep);

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
	static uint16 ReadAndAdvanceBuffer(const uint8*& Buffer, T& Data)
	{
		Data = *(T*)Buffer;
		Buffer += sizeof(T);

		return sizeof(T);
	}

private:
	BufferType m_Buffer;
	uint16 m_ExpectedBytesSize;
};

#endif