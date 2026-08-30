#pragma once
#ifndef DAISY_USB_CDC_INTERFACE_H
#define DAISY_USB_CDC_INTERFACE_H

#include "DaisySeedFramework/USB/USBCDCDefinitions.h"
#include <DigitalSignalProcessing/USB/IUSBCDCInterface.h>
#include <DigitalSignalProcessing/USB/USBProfile.h>

class DaisyUSB;

class DaisyUSBCDCInterface : public IUSBCDCInterface
{
	friend class DaisyUSB;

private:
	DaisyUSBCDCInterface(DaisyUSB* USB);

	bool OnSetupStage(const USBDeviceSetupPacket* Setup);

	static void BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, uint16& Offset, uint8& InterfaceIndex, uint8 Interval, const CDCClassConfig& Config);

private:
	DaisyUSB* m_USB;
	USBCDCLineCoding m_CDCLineCoding;
};

#endif