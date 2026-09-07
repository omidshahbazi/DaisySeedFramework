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

	void Start(uint8 Index, const CDCClassConfig& Config);
	void Stop(void);
	void Update(void);

	bool IsConnected(void) const override
	{
		return m_IsClientConnected;
	}

	void SetReceiveCallback(ReceiveCallback Callback) override
	{
		m_Callback = Callback;
	}

	void Transmit(const uint8* Buffer, uint16 Length) override;

private:
	void Disconnect(void);

	void ListenForClient(void);

private:
	void* m_Pipe;
	std::atomic_bool m_IsRunning;
	std::thread m_ListenThread;
	std::atomic_bool m_IsClientConnected;
	ReceiveCallback m_Callback;
};

#endif