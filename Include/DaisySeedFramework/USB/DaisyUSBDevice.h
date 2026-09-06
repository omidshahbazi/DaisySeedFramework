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

	IUSBInterface* GetInterface(uint8 Index) override
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

		ASSERT(false, "Class is unhandled");
	}

private:
	void OnHALHandleRequest(void);
	void OnSetupStage(void);
	void OnDataOutStage(uint8 EPNum);
	void OnDataInStage(uint8 EPNum);
	void OnIsoOutIncomplete(uint8 EPNum);
	void OnIsoInIncomplete(uint8 EPNum);
	void OnStartOfFrame(void);

	void HandleGetDescriptor(void);

	void AllocateReceiveBuffer(uint16 Size);
	void AllocateTransmitBuffer(uint8 Endpoint, uint16 Size);

	void OpenEndpoint(uint8 Endpoint, uint16 Length, USBEndpointAttributes Type);
	void CloseEndpoint(uint8 Endpoint);

	uint16 DeviceReceiveCount(uint8 Endpoint = USB_EP0_OUT);
	void DeviceReceive(uint8* Buffer, uint16 Length, uint8 Endpoint = USB_EP0_OUT);
	template<typename T>
	void DeviceReceive(T* Buffer)
	{
		DeviceReceive(reinterpret_cast<uint8*>(Buffer), sizeof(T));
	}

	void DeviceReceiveAck(void)
	{
		DeviceReceive(nullptr, 0);
	}

	void DeviceTransmit(const uint8* Buffer, uint16 Length, uint8 Endpoint = USB_EP0_IN, bool ClearDCache = false);
	template<typename T>
	void DeviceTransmit(T* Buffer)
	{
		DeviceTransmit(reinterpret_cast<uint8*>(Buffer), sizeof(T));
	}

	void DeviceTransmitAck(void)
	{
		DeviceTransmit(nullptr, 0);
	}

	void FlushEndpoint(uint8 Endpoint = USB_EP0_IN);

	void SetStall(void);

	DeviceInstanceInfo& GetDeviceInstanceByInterfaceIndex(uint8 InterfaceIndex);
	DeviceInstanceInfo& GetDeviceInstanceByEndpoint(uint8 Endpoint);

	uint16 BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, const USBDeviceProfile& profile);

	static uint16 BuildDeviceDescriptor(EP0Buffer& EP0Buffer, const USBDeviceProfile& Profile);
	static uint16 BuildStringDescriptor(EP0Buffer& EP0Buffer, cstr Value);

private:
	Peripherals m_Peripheral;
	bool m_IsRunning;
	USBDeviceProfile m_Profile;

	PCD_HandleTypeDef m_DeviceHandle;

	BufferTransmitHandler m_EP0TransmitHandler;

	DeviceInstanceInfo m_Devices[MaxClassCount];
	uint8 m_DeviceCount;
};

#endif