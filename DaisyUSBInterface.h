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
	DaisyUSBInterface(daisy::DaisySeed *Hardware)
		: m_Hardware(Hardware)
	{
	}

private:
	void Start(bool TransmissionMode, bool WaitForDebugger)
	{
   		m_Hardware->StartLog(WaitForDebugger);
		
		if (TransmissionMode)
			m_Hardware->usb_handle.SetReceiveCallback(Callback, daisy::UsbHandle::UsbPeriph::FS_INTERNAL);
	}

	void Update(void)
	{
		while(!GetStack()->IsEmpty())
        {
           auto msg = GetStack()->PopFront();
		   
		   m_Callback((const uint8*)msg.Cstr(), msg.Size());
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
	static void Callback(uint8_t *Buffer, uint32_t *Length)
	{
		if(Buffer && Length)
		{
			daisy::FixedCapStr<128> rx((const char *)Buffer, *Length);
			GetStack()->PushBack(rx);
		}
	}

	static daisy::FIFO<daisy::FixedCapStr<128>, 16> *GetStack(void)
	{
		static daisy::FIFO<daisy::FixedCapStr<128>, 16> stack;

		return &stack;
	}

private:
	daisy::DaisySeed *m_Hardware;
	EventHandler m_Callback;
};

#endif