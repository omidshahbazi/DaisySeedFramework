#pragma once
#ifndef WINDOWS_USB_INTERFACE_H
#define WINDOWS_USB_INTERFACE_H

#include "DSP/IUSBInterface.h"

#undef ns
#undef ms
#include <Windows.h>
#undef ns
#undef ms
#define ns *0.000'001
#define ms *0.001

class WindowsUSBInterface : public IUSBInterface
{
	friend class WindowsHAL;

private:
	WindowsUSBInterface(void)
	{}

private:
	void Start(void)
	{
		m_Pipe = CreateFile(L"\\\\.\\pipe\\COM8", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, nullptr);
	}

	void Update(void)
	{
		DWORD bytesAvailable = 0;

		if (PeekNamedPipe(m_Pipe, nullptr, 0, nullptr, &bytesAvailable, nullptr) && bytesAvailable > 0) {

			uint8 buffer[1024];
			DWORD bytesRead;

			if (ReadFile(m_Pipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr)) 
			{
				m_Callback(buffer, bytesRead);
			}
		}
	}

public:
	void Transmit(const uint8* Buffer, uint16 Length) const override
	{
		WriteOnPort(Buffer, Length);
	}

	void SetCallback(EventHandler Callback) override
	{
		m_Callback = Callback;
	}

private:
	void WriteOnPort(const uint8* Buffer, uint16 Length, uint16 Delay = 100) const
	{
		uint16 index = 0;
		while (index < Length)
		{
			const uint16 CountPerStep = 64;

			uint16 countPerStep = (uint16)Math::Min(CountPerStep, Length - index);

			Buffer += index;

			DWORD bytesWritten;
			WriteFile(m_Pipe, Buffer, Length, &bytesWritten, NULL);

			index += CountPerStep;
		}
	}

private:
	HANDLE m_Pipe;
	EventHandler m_Callback;
};

#endif