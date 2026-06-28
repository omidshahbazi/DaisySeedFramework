#pragma once
#ifndef DAISY_USB_INTERFACE_H
#define DAISY_USB_INTERFACE_H

#include "DSP/IUSBInterface.h"
#include "StaticVector.h"
#include <daisy_seed.h>

enum DaisyUSBInterfacePeripherals : uint8
{
	Internal = 0,
	External
};

template <DaisyUSBInterfacePeripherals Peripheral = DaisyUSBInterfacePeripherals::External, uint16 BufferSize = 1024>
class DaisyUSBInterface : public IUSBInterface
{
	friend class DaisySeedHAL;

private:
	typedef StaticVector<uint8, BufferSize> BufferType;

private:
	DaisyUSBInterface(daisy::DaisySeed *Hardware)
		: m_Hardware(Hardware)
	{
	}

private:
	void Start(void)
	{
		new (GetBuffer()) BufferType();

		daisy::UsbHandle::UsbPeriph periph = daisy::UsbHandle::UsbPeriph::FS_INTERNAL;
		if constexpr (Peripheral == DaisyUSBInterfacePeripherals::External)
			periph = daisy::UsbHandle::UsbPeriph::FS_EXTERNAL;

		m_Hardware->usb_handle.Init(periph);
		m_Hardware->usb_handle.SetReceiveCallback(Callback, periph);
	}

	void Update(void)
	{
		static BufferType &buffer = *GetBuffer();
		if (buffer.IsEmpty())
			return;

		m_Callback(buffer.GetData(), buffer.GetSize());

		buffer.Clear();
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
	void WriteOnPort(const uint8 *Buffer, uint16 Length, uint16 Delay = 100) const
	{
		uint16 index = 0;
		while (index < Length)
		{
			const uint16 CountPerStep = 64;

			uint16 countPerStep = (uint16)Math::Min(CountPerStep, Length - index);

			if constexpr (Peripheral == DaisyUSBInterfacePeripherals::Internal)
				m_Hardware->usb_handle.TransmitInternal(const_cast<uint8 *>(Buffer + index), countPerStep);
			else
				m_Hardware->usb_handle.TransmitExternal(const_cast<uint8 *>(Buffer + index), countPerStep);

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
	EventHandler m_Callback;
};

#endif