#pragma once
#ifndef DAISY_USB_CDC_INTERFACE_H
#define DAISY_USB_CDC_INTERFACE_H

#include "DaisySeedFramework/USB/DaisyUSBInterfaceCommon.h"
#include <DigitalSignalProcessing/USB/IUSBCDCInterface.h>
#include <DigitalSignalProcessing/USB/USBProfile.h>

class DaisyUSBCDCInterface : public IUSBCDCInterface, public DaisyUSBInterfaceCommon
{
	friend class DaisyUSB;

private:
	static constexpr uint8 RequiredInterfaceCount = 2;

private:
	DaisyUSBCDCInterface(DaisyUSB* USB, uint16 InterfaceIndexMask, uint8 EndpointCommand, uint8 EndpointIn, uint8 EndpointOut);

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

	void OnDataInStage(void) override;
	void OnDataOutStage(void) override;

	void OnReady(void) override;

	static void BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, uint16& Offset, uint8 InterfaceIndex, uint8 Interval, const CDCClassConfig& Config);

private:
	USBCDCLineCoding m_CDCLineCoding;
	uint8 m_LineState;
	bool m_IsHostConnected;
	uint8 m_ReceiveBuffer[MaxPacketSize];
	ReceiveCallback m_ReceiveCallback;
	BufferTransmitHandler<MaxPacketSize> m_TransmitHandler;
};

#endif