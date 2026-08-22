#pragma once
#ifndef DAISY_USB_INTERFACE_H
#define DAISY_USB_INTERFACE_H

#include <DigitalSignalProcessing/IUSBInterface.h>
#include "StaticVector.h"
#include <daisy_seed.h>

template <bool External = true, uint16 BufferSize = 1024>
class DaisyUSBInterface : public IUSBInterface
{
	friend class DaisySeedHAL;

private:
	typedef StaticVector<uint8, BufferSize> BufferType;

private:
	DaisyUSBInterface(daisy::DaisySeed *Hardware)
		: m_Hardware(Hardware),
		  m_IsStarted(false)
	{
	}

	void Start(void) override
	{
		ASSERT(!m_IsStarted, "It's already started");

		new (GetBuffer()) BufferType();

		daisy::UsbHandle::UsbPeriph periph = daisy::UsbHandle::UsbPeriph::FS_INTERNAL;
		if constexpr (External)
			periph = daisy::UsbHandle::UsbPeriph::FS_EXTERNAL;

		m_Hardware->usb_handle.Init(periph);
		m_Hardware->usb_handle.SetReceiveCallback(Callback, periph);

		m_IsStarted = true;
	}

	void Stop(void) override
	{
		ASSERT(m_IsStarted, "It's not started");

		daisy::UsbHandle::UsbPeriph periph = daisy::UsbHandle::UsbPeriph::FS_INTERNAL;
		if constexpr (External)
			periph = daisy::UsbHandle::UsbPeriph::FS_EXTERNAL;

		m_Hardware->usb_handle.SetReceiveCallback(nullptr, periph);
		m_Hardware->usb_handle.DeInit(periph);

		HandleIncomings();

		m_IsStarted = false;
	}

	void Update(void)
	{
		if (!m_IsStarted)
			return;

		HandleIncomings();
	}

public:
	void Transmit(const uint8 *Buffer, uint16 Length) const override
	{
		WriteOnPort(Buffer, Length);
	}

	void SetCallback(EventHandler Callback) override
	{
		m_Callback = Callback;
	}

private:
	void HandleIncomings(void)
	{
		static BufferType &buffer = *GetBuffer();
		if (buffer.IsEmpty())
			return;

		if (m_Callback != nullptr)
			m_Callback(buffer.GetData(), buffer.GetSize());

		buffer.Clear();
	}

	void WriteOnPort(const uint8 *Buffer, uint16 Length, uint16 Delay = 100) const
	{
		uint16 index = 0;
		while (index < Length)
		{
			const uint16 CountPerStep = 64;

			uint16 countPerStep = (uint16)Math::Min(CountPerStep, Length - index);

			if constexpr (External)
				m_Hardware->usb_handle.TransmitExternal(const_cast<uint8 *>(Buffer + index), countPerStep);
			else
				m_Hardware->usb_handle.TransmitInternal(const_cast<uint8 *>(Buffer + index), countPerStep);

			index += CountPerStep;

			m_Hardware->DelayMs(Delay);
		}
	}

	static void Callback(uint8 *Buffer, uint32_t *Length)
	{
		if (Buffer == nullptr)
			return;

		if (*Length == 0)
			return;

		static BufferType &buffer = *GetBuffer();
		buffer.Clear();

		buffer.PushBack(Buffer, *Length);
	}

	static BufferType *GetBuffer(void)
	{
		static BufferType *buffer = Memory::Allocate<BufferType>(true);

		return buffer;
	}

private:
	daisy::DaisySeed *m_Hardware;
	bool m_IsStarted;
	EventHandler m_Callback;
};

#endif