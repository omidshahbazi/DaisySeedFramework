#pragma once
#ifndef DAISY_USB_HOST_H
#define DAISY_USB_HOST_H

#include "../Common.h"
#include "DaisySeedFramework/USB/USBDefinitions.h"
#include "DaisySeedFramework/DaisyInclude.h"
#include <DigitalSignalProcessing/USB/IUSBHost.h>

extern "C"
{
}

class DaisyUSBHost : public IUSBHost
{
	friend class DaisyUSB;

public:
	DaisyUSBHost(Peripherals Peripheral);

	void Start(const USBHostProfile& Profile);
	void Stop(void);

private:
	void OnHALHandleRequest(void);

private:
	Peripherals m_Peripheral;
	bool m_IsRunning;
	USBHostProfile m_Profile;

	HCD_HandleTypeDef m_HostHandle;
};

#endif