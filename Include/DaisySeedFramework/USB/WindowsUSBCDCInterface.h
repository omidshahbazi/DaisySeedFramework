#pragma once
#ifndef WINDOWS_USB_CDC_INTERFACE_H
#define WINDOWS_USB_CDC_INTERFACE_H

#include <DigitalSignalProcessing/USB/IUSBCDCInterface.h>
#include <DigitalSignalProcessing/USB/USBProfile.h>
#include <thread>
#include <atomic>

class WindowsUSBCDCInterface : public IUSBCDCInterface
{
public:
	WindowsUSBCDCInterface(void);

	void Start(uint8_t Index, const CDCClassConfig& Config);
	void Stop(void);
	void Update(void);

	bool IsConnected(void) const override
	{
		return m_IsClientConnected;
	}

	void SetConnectionStateChangedCallback(StateChangedCallback Callback) override
	{
		m_ConnectionStateChangedCallback = Callback;
	}

	TransmitStates GetTransmitState(void) const
	{
		return m_TransmitState;
	}

	void SetTransmitStateChangedCallback(StateChangedCallback Callback) override
	{
		m_TransmitStateChangedCallback = Callback;
	}

	void SetReceiveCallback(ReceiveCallback Callback) override
	{
		m_Callback = Callback;
	}

	void Transmit(const uint8_t* Buffer, uint16_t Length) override;

private:
	void Disconnect(void);

	void ListenForClient(void);

private:
	CDCClassConfig m_Config;
	void* m_Pipe;
	std::atomic_bool m_IsRunning;
	std::thread m_ListenThread;
	std::atomic_bool m_IsClientConnected;
	StateChangedCallback m_ConnectionStateChangedCallback;
	TransmitStates m_TransmitState;
	StateChangedCallback m_TransmitStateChangedCallback;
	ReceiveCallback m_Callback;
};

#endif