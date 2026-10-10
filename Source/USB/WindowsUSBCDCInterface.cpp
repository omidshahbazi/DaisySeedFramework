#ifdef ON_WINDOWS

#include "DaisySeedFramework/USB/WindowsUSBCDCInterface.h"
#include "DaisySeedFramework/WindowsInclude.h"
#include <DigitalSignalProcessing/Debug.h>
#include <DigitalSignalProcessing/Math.h>
#include <DaisySeedFramework/StaticString.h>

WindowsUSBCDCInterface::WindowsUSBCDCInterface(void)
	: m_Config{},
	m_Pipe(INVALID_HANDLE_VALUE),
	m_IsClientConnected(false),
	m_TransmitState(TransmitStates::Idle)
{}

void WindowsUSBCDCInterface::Start(uint8_t Index, const CDCClassConfig& Config)
{
	ASSERT(!m_IsRunning, "Already started");

	m_Config = Config;

	StaticString<128> name = "WindowsUSBCDCInterface-";
	name += ToString(Index);

	StaticWString<256> path = L"\\\\.\\pipe\\USB-PIPE-";
	path += StaticWString<128>(name.GetValue(), name.GetValue() + name.GetLength());

	m_Pipe = CreateNamedPipeW(path.GetValue(), PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, (uint8_t)m_Config.SendBufferSize, (uint8_t)m_Config.ReceiveBufferSize, 0, nullptr);

	ListenForClient();
}

void WindowsUSBCDCInterface::Stop(void)
{
	ASSERT(m_IsRunning, "Already stopped");

	Disconnect();

	CloseHandle(m_Pipe);

	m_Pipe = INVALID_HANDLE_VALUE;

	m_IsClientConnected = false;

	m_ConnectionStateChangedCallback();
}

void WindowsUSBCDCInterface::Update(void)
{
	if (!m_IsClientConnected)
		return;

	DWORD bytesAvailable = 0;
	if (PeekNamedPipe(m_Pipe, nullptr, 0, nullptr, &bytesAvailable, nullptr))
	{
		if (bytesAvailable == 0)
			return;

		uint8_t buffer[(uint8_t)PacketSizes::Max];
		DWORD bytesToRead = Math::Min(bytesAvailable, (uint8_t)m_Config.ReceiveBufferSize);
		DWORD bytesRead;

		if (ReadFile(m_Pipe, buffer, bytesToRead, &bytesRead, nullptr))
		{
			if (bytesRead != 0 && m_Callback != nullptr)
				m_Callback(buffer, bytesRead);
		}
	}
	else if (GetLastError() == ERROR_BROKEN_PIPE)
		Disconnect();
}

void WindowsUSBCDCInterface::Transmit(const uint8_t* Buffer, uint16_t Length)
{
	if (!m_IsClientConnected)
		return;

	m_TransmitState = TransmitStates::Busy;

	m_TransmitStateChangedCallback();

	uint16_t index = 0;
	while (index < Length)
	{
		const uint8_t CountPerStep = (uint8_t)m_Config.SendBufferSize;

		uint8_t countPerStep = (uint8_t)Math::Min(CountPerStep, Length - index);

		DWORD bytesWritten;
		WriteFile(m_Pipe, Buffer + index, countPerStep, &bytesWritten, NULL);

		index += CountPerStep;
	}

	m_TransmitStateChangedCallback();

	m_TransmitState = TransmitStates::Idle;
}

void WindowsUSBCDCInterface::Disconnect(void)
{
	if (!m_IsClientConnected)
		return;

	DisconnectNamedPipe(m_Pipe);

	m_IsClientConnected = false;

	m_ConnectionStateChangedCallback();
}

void WindowsUSBCDCInterface::ListenForClient(void)
{
	m_IsRunning = true;

	m_ListenThread = std::thread([this]
		{
			while (m_Pipe != INVALID_HANDLE_VALUE)
			{
				if (ConnectNamedPipe(m_Pipe, nullptr))
				{
					m_IsClientConnected = true;

					m_ConnectionStateChangedCallback();
				}

				while (m_IsClientConnected)
					std::this_thread::sleep_for(std::chrono::milliseconds(1));
			}

			m_IsRunning = false;
		});
}

#endif