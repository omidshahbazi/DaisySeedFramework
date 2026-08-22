#pragma once
#ifndef USB_HOST_H_
#define USB_HOST_H_

#include "Common.h"
#include "StaticVector.h"
#include <DigitalSignalProcessing/IHAL.h>

template <uint16 BufferSize>
class USBHost
{
private:
	typedef StaticVector<uint8, BufferSize> BufferType;

public:
	USBHost(IHAL *HAL)
		: m_HAL(HAL),
		  m_USBInterface(HAL->GetUSBInterface()),
		  m_Buffer(nullptr),
		  m_ExpectedBytesSize(0)
	{
	}

	virtual void Initialize(void)
	{
		m_USBInterface->Start();

		m_Buffer = Memory::Allocate<BufferType>(1, true);
		new (m_Buffer) BufferType();

		m_USBInterface->SetCallback({this, [](void *Context, const uint8 *Buffer, uint16 Length)
									 {
										 static_cast<USBHost *>(Context)->OnDataReceived(Buffer, Length);
									 }});
	}

	virtual void Deinitialize(void)
	{
		m_USBInterface->SetCallback(nullptr);

		m_USBInterface->Stop();
	}

protected:
	void Transmit(const uint8 *Buffer, uint16 Length) const
	{
		m_USBInterface->Transmit(reinterpret_cast<const uint8 *>(&Length), sizeof(Length));
		m_USBInterface->Transmit(reinterpret_cast<const uint8 *>(Buffer), Length);
	}

	virtual void HandlePacket(const uint8 *Buffer, uint16 Length) = 0;

private:
	void OnDataReceived(const uint8 *Buffer, uint16 Length)
	{
		m_Buffer->PushBack(Buffer, Length);

		const uint8 HeaderSize = sizeof(m_ExpectedBytesSize);

		while (m_Buffer->GetSize() >= HeaderSize)
		{
			const uint8 *buffer = m_Buffer->GetData();

			ReadAndAdvanceBuffer(buffer, m_ExpectedBytesSize);

			const uint16 ExpectedBufferSize = HeaderSize + m_ExpectedBytesSize;

			if (m_Buffer->GetSize() > ExpectedBufferSize)
			{
				ResetAssembler();
				break;
			}

			if (m_Buffer->GetSize() < ExpectedBufferSize)
				break;

			HandlePacket(buffer, m_ExpectedBytesSize);

			m_Buffer->RemoveRange(0, ExpectedBufferSize);
		}
	}

	void ResetAssembler(void)
	{
		m_Buffer->Clear();
		m_ExpectedBytesSize = 0;
	}

protected:
	template <typename T>
	static uint16 ReadAndAdvanceBuffer(const uint8 *&Buffer, T &Data)
	{
		Data = *(T *)Buffer;
		Buffer += sizeof(T);

		return sizeof(T);
	}

private:
	IHAL *m_HAL;
	IUSBInterface *m_USBInterface;

	BufferType *m_Buffer;
	uint16 m_ExpectedBytesSize;
};

#endif