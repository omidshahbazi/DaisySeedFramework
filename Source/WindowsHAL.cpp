#ifdef ON_WINDOWS

#include "DaisySeedFramework/WindowsHAL.h"
#include "DaisySeedFramework/WindowsInclude.h"
#include <DigitalSignalProcessing/Math.h>
#include <DigitalSignalProcessing/Debug.h>
#include <DigitalSignalProcessing/Memory.h>
#include <portaudio.h>

#pragma comment(lib, "portaudio.lib")

WindowsHAL::WindowsHAL(void* SDRAMAddress, uint32_t SDRAMSize)
	: m_Firmware(this),
	m_Allocator(SDRAMAddress, SDRAMSize),
	m_AudioCallback(nullptr)
{
	m_StartupTime = std::chrono::steady_clock::now();
}

void WindowsHAL::Setup(uint8_t FrameLength, uint32_t SampleRate, bool Boost)
{
	ASSERT(FrameLength != 0, "Invalid FrameLength %i", FrameLength);

	Pa_Initialize();

	PaStream* stream;
	Pa_OpenDefaultStream(&stream, 2, 2, paFloat32, SampleRate, FrameLength, (PaStreamCallback*)AudioCallback, this);

	Pa_StartStream(stream);
}

void* WindowsHAL::Allocate(uint32_t Size, bool OnSDRAM)
{
	if (OnSDRAM)
		return m_Allocator.Allocate(Size);

	return malloc(Size);
}

void WindowsHAL::Deallocate(void* Memory)
{
	if (m_Allocator.Contains(Memory))
		return;

	free(Memory);
}

uint32_t WindowsHAL::GetTimeSinceStartupTicks(void) const
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - m_StartupTime).count();
}

uint32_t WindowsHAL::GetTimeSinceStartupMs(void) const
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - m_StartupTime).count();
}


void WindowsHAL::Reset(bool InfiniteTime) const
{
	std::exit(0);
}

void WindowsHAL::Delay(uint16_t Ms) const
{
	_Thrd_sleep_for(Ms);
}

void WindowsHAL::Update(void)
{
	m_USB.Update();
}

int32_t WindowsHAL::AudioCallback(const void* InputBuffer, void* OutputBuffer, uint32_t FramesPerBuffer, const PaStreamCallbackTimeInfo* TimeInfo, uint32_t StatusFlags, void* UserData)
{
	float* in = (float*)InputBuffer;
	float* out = (float*)OutputBuffer;

	if (InputBuffer == nullptr)
	{
		for (uint32_t i = 0; i < FramesPerBuffer; i++)
			*out++ = 0;

		return paContinue;
	}

	static float inputBuffer[2][256];
	static float outputBuffer[2][256];

	for (uint32_t i = 0; i < FramesPerBuffer; i++)
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

	for (uint32_t i = 0; i < FramesPerBuffer; i++)
	{
		*out++ = outputBuffer[0][i];
		*out++ = outputBuffer[1][i];
	}

	return paContinue;
}

#endif