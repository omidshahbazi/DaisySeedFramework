#pragma once
#ifndef DAISY_USB_INTERFACE_H
#define DAISY_USB_INTERFACE_H

#include "../Common.h"
#include "DaisySeedFramework/DaisyInclude.h"
#include <DigitalSignalProcessing/USB/IUSBInterface.h>

struct EP0Buffer;

extern "C"
{
	void OTG_FS_IRQHandler(void);
	void OTG_HS_IRQHandler(void);
	void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef* hpcd);
	void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum);
	void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum);
}

class DaisyUSBInterface : public IUSBInterface
{
	friend void OTG_FS_IRQHandler(void);
	friend void OTG_HS_IRQHandler(void);
	friend void HAL_PCD_SetupStageCallback(PCD_HandleTypeDef* hpcd);
	friend void HAL_PCD_DataInStageCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum);
	friend void HAL_PCD_DataOutStageCallback(PCD_HandleTypeDef* hpcd, uint8_t epnum);

public:
	enum class Peripherals
	{
		FullSpeed = 0,
		Internal = FullSpeed,

		HighSpeed,
		External = HighSpeed,

		COUNT
	};

private:
	BEGIN_PACK(1);
	struct LineCoding
	{
	public:
		uint32 dwDTERate;
		uint8  bCharFormat; // 1 Stop bit
		uint8  bParityType; // None
		uint8  bDataBits;   // 8 Data bits
	};
	END_PACK();

public:
	DaisyUSBInterface(Peripherals Peripheral);

	void Start(const USBProfile& Profile);
	void Stop(void);

	ICDCUSB* GetCDC(uint8 Index)
	{
		return nullptr;
	}
	IAMCUSB* GetAMC(void)
	{
		return nullptr;
	}
	IMIDIUSB* GetMIDI(void)
	{
		return nullptr;
	}

private:
	void OnHALHandleRequest(void);
	void OnSetupStage(void);
	void OnDataInStage(uint8 EPNum);
	void OnDataOutStage(uint8 EPNum);

	void HandleGetDescriptor(void);

	uint16 BuildDeviceDescriptor(EP0Buffer& EP0Buffer);
	uint16 BuildStringDescriptor(EP0Buffer& EP0Buffer, cstr Value);
	uint16 BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, const USBDeviceProfile& profile);

	void DeviceReceive(uint8* Buffer, uint16 Length);
	template<typename T>
	void DeviceReceive(T* Buffer)
	{
		DeviceReceive(reinterpret_cast<uint8*>(Buffer), sizeof(T));
	}

	void DeviceWaitForReceive(void)
	{
		DeviceReceive(nullptr, 0);
	}

	void DeviceTransmit(uint8* Buffer, uint16 Length);
	template<typename T>
	void DeviceTransmit(T* Buffer)
	{
		DeviceTransmit(reinterpret_cast<uint8*>(Buffer), sizeof(T));
	}

	void DeviceTransmitAck(void)
	{
		DeviceTransmit(nullptr, 0);
	}

private:
	Peripherals m_Peripheral;
	bool m_IsRunning;
	USBProfile m_Profile;

	union
	{
		PCD_HandleTypeDef m_DeviceHandle;
		HCD_HandleTypeDef m_HostHandle;
	};

	LineCoding m_DeviceLineCoding;

	uint16 m_EO0TransmitRemainingLength;
	const uint8* m_EP0TransmitBufferStart;
};

#endif