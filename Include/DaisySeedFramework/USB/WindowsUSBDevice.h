#pragma once
#ifndef WINDOWS_USB_DEVICE_H
#define WINDOWS_USB_DEVICE_H

#include "WindowsUSBCDCInterface.h"
#include "DaisySeedFramework/StaticVector.h"
#include <DigitalSignalProcessing/USB/IUSBDevice.h>
#include <DigitalSignalProcessing/Debug.h>

class WindowsUSBDevice : public IUSBDevice
{
public:
	WindowsUSBDevice(void);

	void Start(uint8_t Index, const USBDeviceProfile& Profile);
	void Stop(void);
	void Update(void);

	IUSBInterface* GetInterface(uint8_t Index) override
	{
		return &m_Interfaces[Index];
	}

private:
	StaticVector<WindowsUSBCDCInterface, MaxClassCount> m_Interfaces;
};

#endif