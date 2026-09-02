#pragma once
#ifndef DAISY_USB_AMC_INTERFACE_H
#define DAISY_USB_AMC_INTERFACE_H

#include "DaisySeedFramework/USB/DaisyUSBInterfaceCommon.h"
#include "DaisySeedFramework/USB/USBAMCDefinitions.h"
#include <DigitalSignalProcessing/USB/IUSBAMCInterface.h>

class DaisyUSBAMCInterface : public IUSBAMCInterface, public DaisyUSBInterfaceCommon
{
public:
	DaisyUSBAMCInterface(DaisyUSB* USB, const Configs& Configs, const AMCClassConfig& Class);

	bool OnSetupStage(const USBDeviceSetupPacket* Setup) override;
	void OnSetupCompleted(void) override;
	void OnDataInStage(void) override;
	void OnDataOutStage(void) override;
	bool OnSetInterface(uint8 InterfaceIndex, uint8 AlternateSetting) override;
	uint8 GetCurrentAltSetting(uint8 InterfaceIndex) const override;
	void BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, uint16& BufferOffset, uint8 InterfaceIndex, const USBClassNode& Class) override;

public:
	void TransmitBuffer(void);

	bool IsSampleRateSupported(uint32 Rate) const;

	static void BuildStreamingInterface(EP0Buffer& EP0Buffer, uint16& BufferOffset, uint8 InterfaceIndex, uint8 ChannelCount, uint8 Endpoint, uint8 TerminalLinkID, const AMCClassConfig& Config, bool IsOutput);

	static void CalculateStreamingInterfaceIndices(const Configs& Configs, const AMCClassConfig& Class, uint8& OutInterfaceIndex, uint8& InInterfaceIndex);
	static uint8 CalculateRequiredInterfaceCount(const AMCClassConfig& Class);

	static uint16 CalculateIsoPacketSize(uint8 ChannelCount, const AMCClassConfig& Class);

private:
	AMCClassConfig m_Class;
	uint32 m_CurrentSampleRate;

	uint8 m_OutAltSetting;
	uint8 m_InAltSetting;

	uint8 m_OutInterfaceIndex;
	uint8 m_InInterfaceIndex;

	uint8* m_TransmitBuffer;
};

#endif