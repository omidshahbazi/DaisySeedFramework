#pragma once
#ifndef DAISY_USB_AMC_INTERFACE_H
#define DAISY_USB_AMC_INTERFACE_H

#include "DaisySeedFramework/USB/DaisyUSBInterfaceCommon.h"
#include <DigitalSignalProcessing/USB/IUSBAMCInterface.h>
#include <DigitalSignalProcessing/USB/USBProfile.h>

class DaisyUSBAMCInterface : public IUSBAMCInterface, public DaisyUSBInterfaceCommon
{
public:
	DaisyUSBAMCInterface(DaisyUSB* USB, const Configs& Configs);

	bool OnSetupStage(const USBDeviceSetupPacket* Setup) override;
	void OnSetupCompleted(void) override;
	void OnDataInStage(void) override;
	void OnDataOutStage(void) override;

	void BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, uint16& BufferOffset, uint8 InterfaceIndex, const USBClassNode& Class) override;

public:
	static uint8 CalculateRequiredInterfaceCount(const AMCClassConfig& Class);

	static uint16 CalculateIsoPacketSize(uint8 ChannelCount, const AMCClassConfig& Class);
};

#endif