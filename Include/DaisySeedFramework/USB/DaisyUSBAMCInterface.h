#pragma once
#ifndef DAISY_USB_AMC_INTERFACE_H
#define DAISY_USB_AMC_INTERFACE_H

#include "DaisySeedFramework/USB/USBDefinitions.h"
#include <DigitalSignalProcessing/USB/IUSBAMCInterface.h>
#include <DigitalSignalProcessing/USB/USBProfile.h>

class DaisyUSBAMCInterface : public IUSBAMCInterface
{
	friend class DaisyUSBInterface;

private:
	static void BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, uint16& Offset, uint8& InterfaceIndex, uint8 Interval, const CDCClassConfig& Config);
};

#endif