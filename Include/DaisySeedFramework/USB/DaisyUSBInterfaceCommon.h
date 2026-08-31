#pragma once
#ifndef DAISY_USB_INTERFACE_COMMON_H
#define DAISY_USB_INTERFACE_COMMON_H

#include "DaisySeedFramework/USB/USBCDCDefinitions.h"

class DaisyUSB;

class DaisyUSBInterfaceCommon
{
	friend class DaisyUSB;

protected:
	DaisyUSBInterfaceCommon(DaisyUSB* USB, uint16 InterfaceIndexMask, uint8 EndpointCommand, uint8 EndpointIn, uint8 EndpointOut);

	virtual bool OnSetupStage(const USBDeviceSetupPacket* Setup) = 0;

	virtual void OnDataInStage(void) = 0;
	virtual void OnDataOutStage(void) = 0;

	virtual void OnReady(void) = 0;

	void OpenEndpoints(void);

	uint16 EndpointReceiveCount(void);
	void EndpointReceive(uint8* Buffer, uint16 Length);
	void EndpointTransmit(const uint8* Buffer, uint16 Length);

	DaisyUSB* GetUSB(void)
	{
		return m_USB;
	}

	bool MatchByInterfaceIndex(uint8 Index);
	bool MatchByEndpoint(uint8 Endpoint);

private:
	DaisyUSB* m_USB;
	uint16 m_InterfaceIndexMask;
	uint8 m_EndpointCommand;
	uint8 m_EndpointIn;
	uint8 m_EndpointOut;
};

#endif