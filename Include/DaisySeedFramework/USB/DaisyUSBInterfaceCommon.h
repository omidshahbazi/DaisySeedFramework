#pragma once
#ifndef DAISY_USB_INTERFACE_COMMON_H
#define DAISY_USB_INTERFACE_COMMON_H

#include "DaisySeedFramework/USB/USBCDCDefinitions.h"
#include <DigitalSignalProcessing/USB/USBProfile.h>

class DaisyUSB;

class DaisyUSBInterfaceCommon
{
public:
	struct Configs
	{
	public:
		uint8 InterfaceIndexStart;
		uint8 InterfaceIndexCount;
		uint8 EndpointCommand;
		uint8 EndpointOut;
		uint8 EndpointIn;
		uint16 ReceivePacketSize;
		uint16 TransmitPacketSize;
	};

public:
	DaisyUSBInterfaceCommon(DaisyUSB* USB, const Configs& Configs);

	virtual bool OnSetupStage(const USBDeviceSetupPacket* Setup) = 0;
	virtual void OnSetupCompleted(void) = 0;

	virtual void OnDataInStage(void) = 0;
	virtual void OnDataOutStage(void) = 0;

	virtual void BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, uint16& BufferOffset, uint8 InterfaceIndex, const USBClassNode& Class) = 0;

	bool MatchByInterfaceIndex(uint8 Index) const;
	bool MatchByEndpoint(uint8 Endpoint) const;

	const Configs& GetConfigs(void) const
	{
		return m_Configs;
	}

protected:
	void OpenEndpoints(USBEpAttr Mode);

	uint16 EndpointReceiveCount(void);
	void EndpointReceive(uint8* Buffer, uint16 Length);
	void EndpointTransmit(const uint8* Buffer, uint16 Length);

	DaisyUSB* GetUSB(void)
	{
		return m_USB;
	}

private:
	DaisyUSB* m_USB;
	Configs m_Configs;
	uint16 m_InterfaceIndexMask;
};

#endif