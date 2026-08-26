#ifdef ON_WINDOWS

#include "DaisySeedFramework/WindowsHAL.h"
#include <DigitalSignalProcessing/Math.h>
#include <DigitalSignalProcessing/Debug.h>
#include <DigitalSignalProcessing/Memory.h>
#include <portaudio.h>

#undef ns
#undef ms
#include <Windows.h>
#undef ns
#undef ms

#pragma comment(lib, "portaudio.lib")

WindowsHAL::WindowsHAL(void* SDRAMAddress, uint32 SDRAMSize, CrashHandler CrashHandler)
	: m_CrashHandler(CrashHandler),
	m_SDRAMAddress(reinterpret_cast<uint8*>(SDRAMAddress)),
	m_SDRAMSize(SDRAMSize),
	m_LastFreeSDRAMIndex(0),
	m_AudioCallback(nullptr)
{
	ASSERT(SDRAMSize == 0 || SDRAMAddress != nullptr, "SDRAMAddress cannot be null");
	ASSERT(SDRAMAddress == nullptr || SDRAMSize > 0, "SDRAMSize cannot be zero");

	m_StartupTime = std::chrono::steady_clock::now();
}

void WindowsHAL::Setup(uint8 FrameLength, uint32 SampleRate, bool Boost)
{
	ASSERT(FrameLength != 0, "Invalid FrameLength %i", FrameLength);

	Pa_Initialize();

	PaStream* stream;
	Pa_OpenDefaultStream(&stream, 2, 2, paFloat32, SampleRate, FrameLength, (PaStreamCallback*)AudioCallback, this);

	Pa_StartStream(stream);
}

void* WindowsHAL::Allocate(uint32 Size, bool OnSDRAM)
{
	if (OnSDRAM)
	{
		const uint8 ALIGNMENT = 16;

		ASSERT(m_SDRAMAddress != nullptr, "SDRAM is not initialized");
		ASSERT(m_LastFreeSDRAMIndex + Size <= m_SDRAMSize, "Running out of SDRAM");

		uint8* ptr = m_SDRAMAddress + m_LastFreeSDRAMIndex;

		uint8* alignedPtr = reinterpret_cast<uint8*>(((reinterpret_cast<uint64>(ptr) + (ALIGNMENT - 1)) / ALIGNMENT) * ALIGNMENT);

		m_LastFreeSDRAMIndex += (alignedPtr - ptr) + Size;

		return alignedPtr;
	}

	return malloc(Size);
}

void WindowsHAL::Deallocate(void* Memory)
{
	if (m_SDRAMAddress != nullptr && m_SDRAMAddress <= Memory)
		return;

	free(Memory);
}

uint32 WindowsHAL::GetTimeSinceStartupTicks(void) const
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_StartupTime).count();
}

uint32 WindowsHAL::GetTimeSinceStartupMs(void) const
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_StartupTime).count();
}

void WindowsHAL::Print(cstr Value)
{
	printf(Value);
}

bool WindowsHAL::IsDebuggerPresent(void) const
{
	return ::IsDebuggerPresent();
}

void WindowsHAL::Crash(void) const
{
	Delay(1000);

	if (m_CrashHandler != nullptr)
	{
		m_CrashHandler(this);
		return;
	}

	Break();
}

void WindowsHAL::Break(void) const
{
	__debugbreak();
}

void WindowsHAL::Reset(bool InfiniteTime) const
{
	exit(0);
}

void WindowsHAL::Delay(uint16 Ms) const
{
	_Thrd_sleep_for(Ms);
}

void WindowsHAL::Update(void)
{
	m_USBInterface.Update();
}

int WindowsHAL::AudioCallback(const void* InputBuffer, void* OutputBuffer, uint32 FramesPerBuffer, const PaStreamCallbackTimeInfo* TimeInfo, uint32 StatusFlags, void* UserData)
{
	float* in = (float*)InputBuffer;
	float* out = (float*)OutputBuffer;

	if (InputBuffer == nullptr)
	{
		for (unsigned int i = 0; i < FramesPerBuffer; i++)
			*out++ = 0;

		return paContinue;
	}

	static float inputBuffer[2][256];
	static float outputBuffer[2][256];

	for (unsigned int i = 0; i < FramesPerBuffer; i++)
	{
		inputBuffer[0][i] = *in++;
		inputBuffer[1][i] = *in++;

		outputBuffer[0][i] = 0;
		outputBuffer[1][i] = 0;
	}

	static WindowsHAL* hal = reinterpret_cast<WindowsHAL*>(UserData);
	if (hal->m_AudioCallback != nullptr)
	{
		static float* inputBuffers[2] = { inputBuffer[0], inputBuffer[1] };
		static float* outputBuffers[2] = { outputBuffer[0], outputBuffer[1] };

		hal->m_AudioCallback(inputBuffers, outputBuffers, FramesPerBuffer);
	}

	for (unsigned int i = 0; i < FramesPerBuffer; i++)
	{
		*out++ = outputBuffer[0][i];
		*out++ = outputBuffer[1][i];
	}

	return paContinue;
}

#endif