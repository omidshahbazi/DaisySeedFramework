#pragma once
#ifndef WINDOWS_USBE_H
#define WINDOWS_USBE_H

#include "WindowsUSBDevice.h"
#include <DigitalSignalProcessing/USB/IUSB.h>

class WindowsUSB : public IUSB
{
	friend class WindowsHAL;

public:
	WindowsUSB(void);

	void Start(const USBProfile& Profile) override;
	void Stop(void) override;
	void Update(void);

	IUSBDevice* GetDevice(void) override
	{
		return &m_Device;
	}

	IUSBHost* GetHost(void) override
	{
		return nullptr;
	}

private:
	WindowsUSBDevice m_Device;
};

#endif