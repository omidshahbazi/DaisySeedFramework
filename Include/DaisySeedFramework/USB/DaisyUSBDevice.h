#pragma once
#ifndef DAISY_USB_DEVICE_H
#define DAISY_USB_DEVICE_H

#include "../Common.h"
#include "DaisySeedFramework/USB/USBDefinitions.h"
#include "DaisySeedFramework/USB/DaisyUSBCDCInterface.h"
#include "DaisySeedFramework/USB/DaisyUSBAMCInterface.h"
#include "DaisySeedFramework/DaisyInclude.h"
#include <DigitalSignalProcessing/USB/IUSBDevice.h>
#include <DigitalSignalProcessing/Debug.h>

extern "C"
{
	void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef* hpcd);
	void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum);
	void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum);
	void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum);
	void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum);
	void HAL_PCD_SOFCallback(PCD_HandleTypeDef* hpcd);
}

class DaisyUSBDevice : public IUSBDevice
{
	friend void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef* hpcd);
	friend void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum);
	friend void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum);
	friend void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum);
	friend void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum);
	friend void HAL_PCD_SOFCallback(PCD_HandleTypeDef* hpcd);

	friend class DaisyUSB;
	friend class DaisyUSBInterfaceCommon;

private:
	BEGIN_PACK(1);
	struct DeviceInstanceInfo
	{
	public:
		USBDeviceClasses Class;
		DaisyUSBInterfaceCommon* Interface;
	};
	END_PACK();

public:
	DaisyUSBDevice(Peripherals Peripheral);

	void Start(const USBDeviceProfile& Profile);
	void Stop(void);

	IUSBInterface* GetInterface(uint8_t Index) override
	{
		ASSERT(Index < m_DeviceCount, "Index out of range");

		const DeviceInstanceInfo& dii = m_Devices[Index];

		switch (dii.Class)
		{
		case USBDeviceClasses::CDC:
			return static_cast<DaisyUSBCDCInterface*>(dii.Interface);

		case USBDeviceClasses::AMC:
			return static_cast<DaisyUSBAMCInterface*>(dii.Interface);
		}

		NOT_IMPLEMENTED();
	}

private:
	void OnHALHandleRequest(void);
	void OnSetupStage(void);
	void OnDataOutStage(uint8_t EPNum);
	void OnDataInStage(uint8_t EPNum);
	void OnIsoOutIncomplete(uint8_t EPNum);
	void OnIsoInIncomplete(uint8_t EPNum);
	void OnStartOfFrame(void);

	void HandleGetDescriptor(void);

	void AllocateReceiveBuffer(uint16_t Size);
	void AllocateTransmitBuffer(uint8_t Endpoint, uint16_t Size);

	void OpenEndpoint(uint8_t Endpoint, uint16_t Length, USBEndpointAttributes Type);
	void CloseEndpoint(uint8_t Endpoint);

	uint16_t DeviceReceiveCount(uint8_t Endpoint = USB_EP0_OUT);
	void DeviceReceive(uint8_t* Buffer, uint16_t Length, uint8_t Endpoint = USB_EP0_OUT);
	template<typename T>
	void DeviceReceive(T* Buffer)
	{
		DeviceReceive(reinterpret_cast<uint8_t*>(Buffer), sizeof(T));
	}

	void DeviceReceiveAck(void)
	{
		DeviceReceive(nullptr, 0);
	}

	void DeviceTransmit(const uint8_t* Buffer, uint16_t Length, uint8_t Endpoint = USB_EP0_IN, bool ClearDCache = false);
	template<typename T>
	void DeviceTransmit(T* Buffer)
	{
		DeviceTransmit(reinterpret_cast<uint8_t*>(Buffer), sizeof(T));
	}

	void DeviceTransmitAck(void)
	{
		DeviceTransmit(nullptr, 0);
	}

	void FlushEndpoint(uint8_t Endpoint = USB_EP0_IN);

	void SetStall(void);

	DeviceInstanceInfo& GetDeviceInstanceByInterfaceIndex(uint8_t InterfaceIndex);
	DeviceInstanceInfo& GetDeviceInstanceByEndpoint(uint8_t Endpoint);

	uint16_t BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, const USBDeviceProfile& profile);

	static uint16_t BuildDeviceDescriptor(EP0Buffer& EP0Buffer, const USBDeviceProfile& Profile);
	static uint16_t BuildStringDescriptor(EP0Buffer& EP0Buffer, cstr Value);

private:
	Peripherals m_Peripheral;
	bool m_IsRunning;
	USBDeviceProfile m_Profile;

	PCD_HandleTypeDef m_DeviceHandle;

	BufferTransmitHandler m_EP0TransmitHandler;

	DeviceInstanceInfo m_Devices[MaxClassCount];
	uint8_t m_DeviceCount;
};

#endif