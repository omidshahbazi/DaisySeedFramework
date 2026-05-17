#pragma once
#ifndef WINDOWS_USB_INTERFACE_H
#define WINDOWS_USB_INTERFACE_H

#include "DSP/IUSBInterface.h"
#include "StaticVector.h"

class WindowsUSBInterface : public IUSBInterface
{
	friend class WindowsHAL;

private:
	typedef StaticVector<uint8, 1024> BufferType;

private:
	WindowsUSBInterface(void)
	{
	}

private:
	void Start(void)
	{
		//new (GetBuffer()) BufferType();

		//daisy::UsbHandle::UsbPeriph periph = daisy::UsbHandle::UsbPeriph::FS_INTERNAL;
		//if constexpr (Peripheral == DaisyUSBInterfacePeripherals::External)
		//	periph = daisy::UsbHandle::UsbPeriph::FS_EXTERNAL;

		//m_Hardware->usb_handle.Init(periph);
		//m_Hardware->usb_handle.SetReceiveCallback(Callback, periph);
	}

	void Update(void)
	{
		//static BufferType &buffer = *GetBuffer();
		//if (buffer.IsEmpty())
		//	return;

		//m_Callback(buffer.GetData(), buffer.GetSize());

		//buffer.Clear();
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
		//uint16 index = 0;
		//while (index < Length)
		//{
		//	const uint16 CountPerStep = 64;

		//	uint16 countPerStep = (uint16)Math::Min(CountPerStep, Length - index);

		//	Buffer += index;

		//	if constexpr (Peripheral == DaisyUSBInterfacePeripherals::Internal)
		//		m_Hardware->usb_handle.TransmitInternal(const_cast<uint8 *>(Buffer), countPerStep);
		//	else
		//		m_Hardware->usb_handle.TransmitExternal(const_cast<uint8 *>(Buffer), countPerStep);

		//	index += CountPerStep;

		//	m_Hardware->DelayMs(Delay);
		//}
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
	//daisy::DaisySeed *m_Hardware;
	EventHandler m_Callback;
};

#endif