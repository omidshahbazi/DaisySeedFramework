#pragma once
#ifndef DAISY_USB_INTERFACE_COMMON_H
#define DAISY_USB_INTERFACE_COMMON_H

#include "DaisySeedFramework/USB/USBDefinitions.h"
#include <DigitalSignalProcessing/USB/USBProfile.h>
#include <DigitalSignalProcessing/Debug.h>

class DaisyUSBDevice;

class DaisyUSBInterfaceCommon
{
public:
	struct Configs
	{
	public:
		uint8_t InterfaceIndexStart;
		uint8_t InterfaceIndexCount;
		uint8_t EndpointCommand;
		uint8_t EndpointOut;
		uint8_t EndpointIn;
		uint16_t MaxReceivePacketSize;
		uint16_t MaxTransmitPacketSize;
	};

public:
	DaisyUSBInterfaceCommon(DaisyUSBDevice* Device, const Configs& Configs);

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

	virtual bool OnSetInterface(uint8_t InterfaceIndex, uint8_t AlternateSetting) = 0;
	virtual uint8_t GetCurrentAltSetting(uint8_t InterfaceIndex) const = 0;
	virtual void BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, uint16_t& BufferOffset, uint8_t InterfaceIndex) const = 0;
	virtual cstr GetDescriptorString(uint8_t StringIndex) const
	{
		return nullptr;
	}

	bool MatchByInterfaceIndex(uint8_t Index) const;
	bool MatchByEndpoint(uint8_t Endpoint) const;

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

		m_PendingReceiveType = (uint8_t)Type;

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
	void AllocateTransmitBuffer(uint8_t Endpoint, uint16_t Size);

	void OpenEndpoint(uint8_t Endpoint, uint16_t Length, USBEndpointAttributes Type);
	void CloseEndpoint(uint8_t Endpoint);

	void DeviceReceive(uint8_t* Buffer, uint16_t Length, uint8_t Endpoint = USB_EP0_OUT);
	template<typename T>
	void DeviceReceive(T* Buffer)
	{
		DeviceReceive(reinterpret_cast<uint8_t*>(Buffer), sizeof(T));
	}
	void DeviceReceiveAck(void);

	void DeviceTransmit(const uint8_t* Buffer, uint16_t Length, uint8_t Endpoint = USB_EP0_OUT, bool ClearDCache = false);
	template<typename T>
	void DeviceTransmit(T* Buffer)
	{
		DeviceTransmit(reinterpret_cast<uint8_t*>(Buffer), sizeof(T));
	}
	void DeviceTransmitAck(void);
	//------------------------------------------------------

	void EndpointPrepareReceive(uint8_t* Buffer, uint16_t Length)
	{
		EndpointReceive(Buffer, Length);
	}
	uint16_t EndpointReceiveCount(void);
	void EndpointReceive(uint8_t* Buffer, uint16_t Length)
	{
		DeviceReceive(Buffer, Length, m_Configs.EndpointOut);
	}
	void EndpointReceiveFlush(void);

	void EndpointTransmit(const uint8_t* Buffer, uint16_t Length, bool ClearDCache = false)
	{
		DeviceTransmit(Buffer, Length, m_Configs.EndpointIn, ClearDCache);
	}
	void EndpointTransmitFlush(void);

private:
	DaisyUSBDevice* m_Device;
	Configs m_Configs;
	uint16_t m_InterfaceIndexMask;

	uint8_t m_PendingReceiveType;
	uint8_t m_PendingReceiveBuffer[sizeof(uint32_t)];
};

#endif