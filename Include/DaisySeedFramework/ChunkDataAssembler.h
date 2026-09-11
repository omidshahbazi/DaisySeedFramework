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
		: m_ExpectedDataSize(0)
	{}

	void Reset(void)
	{
		ClearTransmitBuffer();
		ResetAssembler();
	}

	void ClearTransmitBuffer(void)
	{
		m_TransmitBuffer.Clear();
	}

	template<typename U>
	void PushToTransmitBuffer(const U& Data)
	{
		m_TransmitBuffer.PushBack(reinterpret_cast<const uint8_t*>(&Data), sizeof(U));
	}

	void PushToTransmitBuffer(const uint8_t* Buffer, uint16_t Length)
	{
		ASSERT(Buffer != nullptr, "Buffer cannot be null");
		ASSERT(Length != 0, "Buffer cannot be null");

		m_TransmitBuffer.PushBack(Buffer, Length);
	}

	void Transmit(void)
	{
		uint16_t len = m_TransmitBuffer.GetSize();

		m_TransmitBuffer.Insert(0, reinterpret_cast<const uint8_t*>(&len), sizeof(uint16_t));

		WriteOnPort(m_TransmitBuffer.GetData(), m_TransmitBuffer.GetSize());
	}

	virtual void WriteOnPort(const uint8_t* Buffer, uint16_t Length) = 0;

	virtual void OnFalseDataReceived(void)
	{}

	void StoreReceivedPacket(const uint8_t* Buffer, uint8_t Length)
	{
		m_ReceiveBuffer.PushBack(Buffer, Length);
	}

	void HandleReceivedPacket(void)
	{
		const uint8_t HeaderSize = sizeof(m_ExpectedDataSize);

		while (m_ReceiveBuffer.GetSize() >= HeaderSize)
		{
			const uint8_t* buffer = m_ReceiveBuffer.GetData();

			ReadAndAdvanceBuffer(buffer, m_ExpectedDataSize);

			const uint16_t ExpectedReceiveSize = HeaderSize + m_ExpectedDataSize;

			if (m_ReceiveBuffer.GetSize() > ExpectedReceiveSize)
			{
				ResetAssembler();

				OnFalseDataReceived();

				break;
			}

			if (m_ReceiveBuffer.GetSize() < ExpectedReceiveSize)
				break;

			OnReceivedPacket(buffer, m_ExpectedDataSize);

			m_ReceiveBuffer.RemoveRange(0, ExpectedReceiveSize);
		}
	}

	virtual void OnReceivedPacket(const uint8_t* Buffer, uint16_t Length) = 0;

private:
	void ResetAssembler(void)
	{
		m_ReceiveBuffer.Clear();
		m_ExpectedDataSize = 0;
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
	BufferType m_ReceiveBuffer;
	BufferType m_TransmitBuffer;
	uint16_t m_ExpectedDataSize;
};

#endif