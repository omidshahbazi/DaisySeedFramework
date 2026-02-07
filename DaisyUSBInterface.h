#pragma once
#ifndef DAISY_USB_INTERFACE_H
#define DAISY_USB_INTERFACE_H

#include "DSP/IUSBInterface.h"
#include <daisy_seed.h>

class DaisyUSBInterface : public IUSBInterface
{
	template <uint16 PersistentSlotCount, uint16 PersistentSlotSize>
	friend class DaisySeedHAL;

private:
	typedef daisy::FixedCapStr<1024> BufferType;
	typedef daisy::FIFO<BufferType, 4> StackType;

private:
	DaisyUSBInterface(daisy::DaisySeed *Hardware)
		: m_Hardware(Hardware)
	{
	}

private:
	void Start()
	{
		m_Hardware->usb_handle.Init(daisy::UsbHandle::UsbPeriph::FS_EXTERNAL);
		
		m_Hardware->usb_handle.SetReceiveCallback(Callback, daisy::UsbHandle::UsbPeriph::FS_EXTERNAL);
	}

	void Update(void)
	{
		while(!GetStack()->IsEmpty())
        {
           const BufferType &buffer = GetStack()->Front();
		   
		   m_Callback((const uint8*)buffer.Cstr(), buffer.Size());
		   
		   GetStack()->PopFront();
        }
	}

public:
	void Transmit(const uint8* Buffer, uint32 Length) override
	{
		m_Hardware->usb_handle.TransmitInternal(const_cast<uint8*>(Buffer), Length);
	}

	void SetCallback(EventHandler Callback) override
	{
		m_Callback = Callback;
	}

private:
	static void Callback(uint8 *Buffer, uint32_t *Length)
	{
		if(Buffer == nullptr)
			return;
			
		if (Length == 0)
			return;

		GetStack()->PushBack(BufferType((const char *)Buffer, *Length));
	}

	static StackType *GetStack(void)
	{
		static StackType stack;

		return &stack;
	}

private:
	daisy::DaisySeed *m_Hardware;
	EventHandler m_Callback;
};

#endif