#pragma once
#ifndef DAISY_USB_H
#define DAISY_USB_H

#include "../Common.h"
#include "DaisySeedFramework/USB/USBDefinitions.h"
#include "DaisySeedFramework/USB/DaisyUSBCDCInterface.h"
#include "DaisySeedFramework/USB/DaisyUSBAMCInterface.h"
#include "DaisySeedFramework/DaisyInclude.h"
#include <DigitalSignalProcessing/USB/IUSB.h>
#include <DigitalSignalProcessing/Debug.h>

extern "C"
{
	void OTG_FS_IRQHandler(void);
	void OTG_HS_IRQHandler(void);
	void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef* hpcd);
	void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum);
	void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum);
	void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum);
	void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum);
	void HAL_PCD_SOFCallback(PCD_HandleTypeDef* hpcd);
}

class DaisyUSB : public IUSB
{
	friend void OTG_FS_IRQHandler(void);
	friend void OTG_HS_IRQHandler(void);
	friend void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef* hpcd);
	friend void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum);
	friend void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum);
	friend void HAL_PCD_ISOOUTIncompleteCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum);
	friend void HAL_PCD_ISOINIncompleteCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum);
	friend void HAL_PCD_SOFCallback(PCD_HandleTypeDef* hpcd);

	friend class DaisyUSBInterfaceCommon;

public:
	enum class Peripherals
	{
		FullSpeed = 0,
		Internal = FullSpeed,

		//It's not actually HighSpeed, but the USB peripheral is capable of running at HighSpeed, and the USB_OTG_HS peripheral is not available on the Daisy Seed.
		HighSpeed,
		External = HighSpeed,

		COUNT
	};

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
	DaisyUSB(Peripherals Peripheral);

	void Start(const USBProfile& Profile);
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

	static uint16 BuildDeviceDescriptor(EP0Buffer& EP0Buffer, const USBProfile& Profile);
	static uint16 BuildStringDescriptor(EP0Buffer& EP0Buffer, cstr Value);

private:
	Peripherals m_Peripheral;
	bool m_IsRunning;
	USBProfile m_Profile;

	union
	{
		PCD_HandleTypeDef m_DeviceHandle;
		HCD_HandleTypeDef m_HostHandle;
	};

	BufferTransmitHandler m_EP0TransmitHandler;

	DeviceInstanceInfo m_Devices[MaxClassCount];
	uint8 m_DeviceCount;
};

#endif