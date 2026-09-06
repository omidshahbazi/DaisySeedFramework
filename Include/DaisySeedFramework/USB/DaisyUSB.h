#pragma once
#ifndef DAISY_USB_H
#define DAISY_USB_H

#include "../Common.h"
#include "DaisySeedFramework/USB/DaisyUSBDevice.h"
#include "DaisySeedFramework/USB/DaisyUSBHost.h"
#include <DigitalSignalProcessing/USB/IUSB.h>

extern "C"
{
	void OTG_FS_IRQHandler(void);
	void OTG_HS_IRQHandler(void);
}

class DaisyUSB : public IUSB
{
	friend void OTG_FS_IRQHandler(void);
	friend void OTG_HS_IRQHandler(void);

public:
	DaisyUSB(Peripherals Peripheral);
	~DaisyUSB(void)
	{}

	void Start(const USBProfile& Profile);
	void Stop(void);

	IUSBDevice* GetDevice(void) override
	{
		return &m_Device;
	}

	IUSBHost* GetHost(void) override
	{
		return &m_Host;
	}

private:
	void OnHALHandleRequest(void);

private:
	Peripherals m_Peripheral;
	bool m_IsRunning;
	USBProfile m_Profile;

	union
	{
		DaisyUSBDevice m_Device;
		DaisyUSBHost m_Host;
	};
};

#endif