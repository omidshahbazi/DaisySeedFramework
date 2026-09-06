#pragma once
#ifndef DAISY_USB_INTERFACE_COMMON_H
#define DAISY_USB_INTERFACE_COMMON_H

#include "DaisySeedFramework/USB/USBDefinitions.h"
#include <DigitalSignalProcessing/USB/USBProfile.h>
#include <DigitalSignalProcessing/Debug.h>

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
		uint16 MaxReceivePacketSize;
		uint16 MaxTransmitPacketSize;
	};

public:
	DaisyUSBInterfaceCommon(DaisyUSB* USB, const Configs& Configs);

	virtual bool OnSetupStage(const USBDeviceSetupPacket* Setup) = 0;
	virtual void OnSetupCompleted(void) = 0;
	virtual void OnDeviceDataOutStage(void)
	{}
	virtual void OnDeviceDataInStage(void)
	{}
	virtual void OnDataOutStage(void)
	{}
	virtual void OnDataInStage(void)
	{}
	virtual void OnIsoOutIncomplete(void)
	{}
	virtual void OnIsoInIncomplete(void)
	{}
	virtual void OnStartOfFrame(void)
	{}

	virtual bool OnSetInterface(uint8 InterfaceIndex, uint8 AlternateSetting) = 0;
	virtual uint8 GetCurrentAltSetting(uint8 InterfaceIndex) const = 0;
	virtual void BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, uint16& BufferOffset, uint8 InterfaceIndex) const = 0;
	virtual cstr GetDescriptorString(uint8 StringIndex) const
	{
		return nullptr;
	}

	bool MatchByInterfaceIndex(uint8 Index) const;
	bool MatchByEndpoint(uint8 Endpoint) const;

	const Configs& GetConfigs(void) const
	{
		return m_Configs;
	}

protected:
	template<typename ControlType>
	ControlType GetPendingType(void) const
	{
		return (ControlType)m_PendingReceiveType;
	}
	template<typename T, typename ControlType>
	void SetPendingReceive(ControlType Type)
	{
		//ASSERT(m_PendingReceiveType == 0, "No pending receive");
		ASSERT(sizeof(T) <= sizeof(m_PendingReceiveBuffer), "Pending receive buffer too small");

		m_PendingReceiveType = (uint8)Type;

		DeviceReceive(m_PendingReceiveBuffer, sizeof(T));
	}
	template<typename T>
	T ReadPendingReceive(void)
	{
		ASSERT(m_PendingReceiveType != 0, "No pending receive");

		T value = *reinterpret_cast<T*>(m_PendingReceiveBuffer);

		m_PendingReceiveType = 0;

		return value;
	}

	// Proxy functions to the underlying DaisyUSB instance
	//------------------------------------------------------
	void AllocateTransmitBuffer(uint8 Endpoint, uint16 Size);

	void OpenEndpoint(uint8 Endpoint, uint16 Length, USBEndpointAttributes Type);
	void CloseEndpoint(uint8 Endpoint);

	void DeviceReceive(uint8* Buffer, uint16 Length, uint8 Endpoint = USB_EP0_OUT);
	template<typename T>
	void DeviceReceive(T* Buffer)
	{
		DeviceReceive(reinterpret_cast<uint8*>(Buffer), sizeof(T));
	}
	void DeviceReceiveAck(void);

	void DeviceTransmit(const uint8* Buffer, uint16 Length, uint8 Endpoint = USB_EP0_OUT, bool ClearDCache = false);
	template<typename T>
	void DeviceTransmit(T* Buffer)
	{
		DeviceTransmit(reinterpret_cast<uint8*>(Buffer), sizeof(T));
	}
	void DeviceTransmitAck(void);
	//------------------------------------------------------

	void EndpointPrepareReceive(uint8* Buffer, uint16 Length)
	{
		EndpointReceive(Buffer, Length);
	}
	uint16 EndpointReceiveCount(void);
	void EndpointReceive(uint8* Buffer, uint16 Length)
	{
		DeviceReceive(Buffer, Length, m_Configs.EndpointOut);
	}
	void EndpointReceiveFlush(void);

	void EndpointTransmit(const uint8* Buffer, uint16 Length, bool ClearDCache = false)
	{
		DeviceTransmit(Buffer, Length, m_Configs.EndpointIn, ClearDCache);
	}
	void EndpointTransmitFlush(void);

private:
	DaisyUSB* m_USB;
	Configs m_Configs;
	uint16 m_InterfaceIndexMask;

	uint8 m_PendingReceiveType;
	uint8_t m_PendingReceiveBuffer[sizeof(uint32_t)];
};

#endif