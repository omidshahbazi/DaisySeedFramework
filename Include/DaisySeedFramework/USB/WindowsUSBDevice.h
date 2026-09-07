#pragma once
#ifndef WINDOWS_USB_DEVICE_H
#define WINDOWS_USB_DEVICE_H

#include "WindowsUSBCDCInterface.h"
#include <DigitalSignalProcessing/USB/IUSBDevice.h>
#include <DigitalSignalProcessing/Debug.h>

class WindowsUSBDevice : public IUSBDevice
{
public:
	WindowsUSBDevice(void);

	void Start(uint8 Index, const USBDeviceProfile& Profile);
	void Stop(void);
	void Update(void);

	IUSBInterface* GetInterface(uint8 Index) override
	{
		ASSERT(Index < m_DeviceCount, "Invalid interface index %i", Index);

		return &m_Interfaces[Index];
	}

private:
	WindowsUSBCDCInterface m_Interfaces[MaxClassCount];
	uint8 m_DeviceCount;
};

#endif