#pragma once
#ifndef DAISY_USB_CDC_INTERFACE_H
#define DAISY_USB_CDC_INTERFACE_H

#include "DaisySeedFramework/USB/DaisyUSBInterfaceCommon.h"
#include <DigitalSignalProcessing/USB/IUSBCDCInterface.h>

class DaisyUSBCDCInterface : public IUSBCDCInterface, public DaisyUSBInterfaceCommon
{
public:
	DaisyUSBCDCInterface(DaisyUSB* USB, const Configs& Configs);

	void SetReceiveCallback(ReceiveCallback Callback) override
	{
		m_ReceiveCallback = Callback;
	}

	void Transmit(const uint8* Buffer, uint16 Length) override;

	bool IsConnected(void) const override
	{
		return m_IsHostConnected;
	}

	bool OnSetupStage(const USBDeviceSetupPacket* Setup) override;
	void OnSetupCompleted(void) override;
	void OnDataInStage(void) override;
	void OnDataOutStage(void) override;

	void BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, uint16& BufferOffset, uint8 InterfaceIndex, const USBClassNode& Class) override;

public:
	static uint8 CalculateRequiredInterfaceCount(const CDCClassConfig& Class)
	{
		return 2;
	}

private:
	USBCDCLineCoding m_CDCLineCoding;
	uint8 m_LineState;
	bool m_IsHostConnected;
	uint8 m_ReceiveBuffer[(uint16)PacketSizes::Max];
	BufferTransmitHandler m_TransmitHandler;
	ReceiveCallback m_ReceiveCallback;
};

#endif