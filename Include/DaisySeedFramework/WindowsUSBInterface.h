#pragma once
#ifndef WINDOWS_USB_INTERFACE_H
#define WINDOWS_USB_INTERFACE_H

#include <DigitalSignalProcessing/IUSBInterface.h>
#include <thread>
#include <atomic>

class WindowsUSBInterface : public IUSBInterface
{
	friend class WindowsHAL;

private:
	WindowsUSBInterface(cstr Name = "Interface");

private:
	void Start(USBInterfaces Interface) override;

	void Stop(void) override;

	void Update(void);

public:
	void Transmit(const uint8* Buffer, uint16 Length) const override;

	void SetCallback(EventHandler Callback) override
	{
		m_Callback = Callback;
	}

private:
	void Disconnect(void);

	void ListenForClient(void);

private:
	cstr m_Name;
	void* m_Pipe;
	std::atomic_bool m_IsRunning;
	std::thread m_ListenThread;
	std::atomic_bool m_IsClientConnected;
	EventHandler m_Callback;
};

#endif