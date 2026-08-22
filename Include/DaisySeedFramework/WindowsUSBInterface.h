#pragma once
#ifndef WINDOWS_USB_INTERFACE_H
#define WINDOWS_USB_INTERFACE_H

#include <DigitalSignalProcessing/IUSBInterface.h>
#include <thread>
#include <atomic>
#include <string>

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
	WindowsUSBInterface(cstr Name = "Interface") :
		m_Name(Name),
		m_Pipe(INVALID_HANDLE_VALUE)
	{}

private:
	void Start(void) override
	{
		ASSERT(!m_IsRunning, "Already started");

		const std::string name = m_Name;
		const std::wstring path = L"\\\\.\\pipe\\USB-PIPE-" + std::wstring(name.begin(), name.end());

		m_Pipe = CreateNamedPipeW(path.c_str(), PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 1024, 1024, 0, nullptr);

		ListenForClient();
	}

	void Stop(void) override
	{
		ASSERT(m_IsRunning, "Already stopped");

		Disconnect();

		CloseHandle(m_Pipe);

		m_Pipe = INVALID_HANDLE_VALUE;

		m_IsClientConnected = false;
	}

	void Update(void)
	{
		if (!m_IsClientConnected)
			return;

		DWORD bytesAvailable = 0;
		if (PeekNamedPipe(m_Pipe, nullptr, 0, nullptr, &bytesAvailable, nullptr))
		{
			if (bytesAvailable == 0)
				return;

			uint8 buffer[1024];
			DWORD bytesRead;

			if (ReadFile(m_Pipe, buffer, sizeof(buffer) - 1, &bytesRead, nullptr))
			{
				if (m_Callback != nullptr)
					m_Callback(buffer, bytesRead);
			}
		}
		else if (GetLastError() == ERROR_BROKEN_PIPE)
			Disconnect();
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
		if (!m_IsClientConnected)
			return;

		uint16 index = 0;
		while (index < Length)
		{
			const uint16 CountPerStep = 64;

			uint16 countPerStep = (uint16)Math::Min(CountPerStep, Length - index);

			DWORD bytesWritten;
			WriteFile(m_Pipe, Buffer + index, countPerStep, &bytesWritten, NULL);

			index += CountPerStep;
		}
	}

	void Disconnect(void)
	{
		if (!m_IsClientConnected)
			return;

		DisconnectNamedPipe(m_Pipe);

		m_IsClientConnected = false;
	}

	void ListenForClient(void)
	{
		m_IsRunning = true;

		m_ListenThread = std::thread([this]
			{
				while (m_Pipe != INVALID_HANDLE_VALUE)
				{
					if (ConnectNamedPipe(m_Pipe, nullptr))
						m_IsClientConnected = true;

					while (m_IsClientConnected)
						std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}

				m_IsRunning = false;
			});
	}

private:
	cstr m_Name;
	HANDLE m_Pipe;
	std::atomic_bool m_IsRunning;
	std::thread m_ListenThread;
	std::atomic_bool m_IsClientConnected;
	EventHandler m_Callback;
};

#endif