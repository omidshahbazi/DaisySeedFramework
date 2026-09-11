#pragma once
#ifndef DAISY_USB_CDC_INTERFACE_H
#define DAISY_USB_CDC_INTERFACE_H

#include "DaisySeedFramework/USB/DaisyUSBInterfaceCommon.h"
#include "DaisySeedFramework/USB/USBCDCDefinitions.h"
#include <DigitalSignalProcessing/USB/IUSBCDCInterface.h>

class DaisyUSBCDCInterface : public IUSBCDCInterface, public DaisyUSBInterfaceCommon
{
public:
	DaisyUSBCDCInterface(DaisyUSBDevice* Device, const Configs& Configs, const CDCClassConfig& Class);

	void SetReceiveCallback(ReceiveCallback Callback) override
	{
		m_ReceiveCallback = Callback;
	}

	void Transmit(const uint8_t* Buffer, uint16_t Length) override;

	bool IsConnected(void) const override
	{
		return m_IsHostConnected;
	}

	void SetConnectionStateChangedCallback(StateChangedCallback Callback) override
	{
		m_ConnectionStateChangedCallback = Callback;
	}

	TransmitStates GetTransmitState(void) const
	{
		if (m_TransmitHandler.HasMore())
			return TransmitStates::Busy;

		return TransmitStates::Idle;
	}

	void SetTransmitStateChangedCallback(StateChangedCallback Callback) override
	{
		m_TransmitStateChangedCallback = Callback;
	}

	bool OnSetupStage(const USBDeviceSetupPacket* Setup) override;
	void OnSetupCompleted(void) override;
	void OnDataInStage(void) override;
	void OnDataOutStage(void) override;

	bool OnSetInterface(uint8_t InterfaceIndex, uint8_t AlternateSetting) override
	{
		return (AlternateSetting == 0);
	}
	uint8_t GetCurrentAltSetting(uint8_t InterfaceIndex) const override
	{
		return 0;
	}
	void BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, uint16_t& BufferOffset, uint8_t InterfaceIndex) const override;

public:
	static uint8_t CalculateRequiredInterfaceCount(const CDCClassConfig& Class)
	{
		return 2;
	}

private:
	CDCClassConfig m_Class;

	USBCDCLineCoding m_CDCLineCoding;
	uint8_t m_LineState;

	bool m_IsHostConnected;
	StateChangedCallback m_ConnectionStateChangedCallback;

	StateChangedCallback m_TransmitStateChangedCallback;

	uint8_t m_ReceiveBuffer[(uint16_t)PacketSizes::Max];
	BufferTransmitHandler m_TransmitHandler;
	ReceiveCallback m_ReceiveCallback;
};

#endif