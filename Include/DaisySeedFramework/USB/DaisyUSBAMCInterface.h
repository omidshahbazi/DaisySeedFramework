#pragma once
#ifndef DAISY_USB_AMC_INTERFACE_H
#define DAISY_USB_AMC_INTERFACE_H

#include "DaisySeedFramework/USB/DaisyUSBInterfaceCommon.h"
#include <DigitalSignalProcessing/USB/IUSBAMCInterface.h>
#include <DigitalSignalProcessing/USB/USBProfile.h>

class DaisyUSBAMCInterface : public IUSBAMCInterface, public DaisyUSBInterfaceCommon
{
	friend class DaisyUSB;

private:
	static constexpr uint8 RequiredInterfaceCount = 1;

private:
	DaisyUSBAMCInterface(DaisyUSB* USB, uint16 InterfaceIndexMask, uint8 EndpointCommand, uint8 EndpointIn, uint8 EndpointOut);

	bool OnSetupStage(const USBDeviceSetupPacket* Setup) override;

	void OnDataOutStage(void) override;

	static void BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, uint16& Offset, uint8& InterfaceIndex, uint8 Interval, const CDCClassConfig& Config);

private:
};

#endif