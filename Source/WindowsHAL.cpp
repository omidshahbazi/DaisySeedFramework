#ifdef ON_WINDOWS

#include "DaisySeedFramework/WindowsHAL.h"
#include "DaisySeedFramework/WindowsInclude.h"
#include "DaisySeedFramework/StaticString.h"
#include <DigitalSignalProcessing/Math.h>
#include <DigitalSignalProcessing/Debug.h>
#include <DigitalSignalProcessing/Memory.h>
#include <portaudio.h>

#pragma comment(lib, "portaudio.lib")

#define CHECK_CALL(Expr) \
	{ \
		PaError error = Expr; \
		ASSERT(error == paNoError, #Expr " call failed with error %s", Pa_GetErrorText(error)); \
	}

void GetStreamParameters(cstr PreferredInputInterfaceName, cstr PreferredOutputInterfaceName, PaStreamParameters& Input, PaStreamParameters& Output)
{
	PaDeviceIndex inputDevice = Pa_GetDefaultInputDevice();
	PaDeviceIndex outputDevice = Pa_GetDefaultOutputDevice();

	PaDeviceIndex numDevices = Pa_GetDeviceCount();
	for (PaDeviceIndex i = 0; i < numDevices; i++)
	{
		const PaDeviceInfo* deviceInfo = Pa_GetDeviceInfo(i);
		const PaHostApiInfo* hostApiInfo = Pa_GetHostApiInfo(deviceInfo->hostApi);

		if (hostApiInfo->type != PaHostApiTypeId::paWASAPI)
			continue;

		if (PreferredInputInterfaceName != nullptr && deviceInfo->maxInputChannels != 0 && strstr(deviceInfo->name, PreferredInputInterfaceName) != nullptr)
			inputDevice = i;

		if (PreferredOutputInterfaceName != nullptr && deviceInfo->maxOutputChannels != 0 && strstr(deviceInfo->name, PreferredOutputInterfaceName) != nullptr)
			outputDevice = i;
	}

	const PaDeviceInfo* inputDeviceInfo = Pa_GetDeviceInfo(inputDevice);
	if (inputDeviceInfo != nullptr)
	{
		Input = {};
		Input.device = inputDevice;
		Input.channelCount = inputDeviceInfo->maxInputChannels;
		Input.sampleFormat = paFloat32;
		Input.suggestedLatency = inputDeviceInfo->defaultLowInputLatency;
	}

	const PaDeviceInfo* outputDeviceInfo = Pa_GetDeviceInfo(outputDevice);
	if (outputDeviceInfo != nullptr)
	{
		Output = {};
		Output.device = outputDevice;
		Output.channelCount = outputDeviceInfo->maxOutputChannels;
		Output.sampleFormat = paFloat32;
		Output.suggestedLatency = outputDeviceInfo->defaultLowOutputLatency;
	}
}

WindowsHAL::WindowsHAL(void* SDRAMAddress, uint32_t SDRAMSize)
	: m_PreferredInputInterface(nullptr),
	m_PreferredOutputInterface(nullptr),
	m_InputChannelCount(0),
	m_OutputChannelCount(0),
	m_Firmware(this),
	m_Allocator(SDRAMAddress, SDRAMSize),
	m_AudioCallback(nullptr)
{
	m_StartupTime = std::chrono::steady_clock::now();
}

void WindowsHAL::Setup(uint8_t FrameLength, uint32_t SampleRate, bool Boost)
{
	ASSERT(FrameLength != 0, "Invalid FrameLength %i", FrameLength);

	CHECK_CALL(Pa_Initialize());

	PaStreamParameters input;
	PaStreamParameters output;
	GetStreamParameters(m_PreferredInputInterface, m_PreferredOutputInterface, input, output);

	m_InputChannelCount = input.channelCount;
	m_OutputChannelCount = output.channelCount;

	PaStream* stream;
	if (m_InputChannelCount == 0)
	{
		CHECK_CALL(Pa_OpenStream(&stream, nullptr, &output, SampleRate, FrameLength, paNoFlag, (PaStreamCallback*)AudioCallback, this));
	}
	else if (m_OutputChannelCount == 0)
	{
		CHECK_CALL(Pa_OpenStream(&stream, &input, nullptr, SampleRate, FrameLength, paNoFlag, (PaStreamCallback*)AudioCallback, this));
	}
	else
	{
		CHECK_CALL(Pa_OpenStream(&stream, &input, &output, SampleRate, FrameLength, paNoFlag, (PaStreamCallback*)AudioCallback, this));
	}

	CHECK_CALL(Pa_StartStream(stream));
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
	static WindowsHAL* hal = reinterpret_cast<WindowsHAL*>(UserData);

	float* output = (float*)OutputBuffer;
	if (output == nullptr)
		return paAbort;

	static float inputBuffer[2][256];
	Memory::Set((float*)inputBuffer, 0, sizeof(inputBuffer) / sizeof(float));

	float* input = (float*)InputBuffer;
	if (input != nullptr)
		if (hal->m_InputChannelCount == 1)
			for (uint32_t i = 0; i < FramesPerBuffer; i++)
				inputBuffer[0][i] = inputBuffer[1][i] = *input++;
		else
			for (uint32_t i = 0; i < FramesPerBuffer; i++)
			{
				inputBuffer[0][i] = *input++;
				inputBuffer[1][i] = *input++;
			}

	static float outputBuffer[2][256];
	Memory::Set((float*)outputBuffer, 0, sizeof(outputBuffer) / sizeof(float));

	if (hal->m_AudioCallback != nullptr)
	{
		static float* inputBuffers[2] = { inputBuffer[0], inputBuffer[1] };
		static float* outputBuffers[2] = { outputBuffer[0], outputBuffer[1] };

		hal->m_AudioCallback(inputBuffers, outputBuffers, FramesPerBuffer);
	}

	if (hal->m_OutputChannelCount == 1)
		for (uint32_t i = 0; i < FramesPerBuffer; i++)
			*output++ = outputBuffer[0][i];
	else
		for (uint32_t i = 0; i < FramesPerBuffer; i++)
		{
			*output++ = outputBuffer[0][i];
			*output++ = outputBuffer[1][i];
		}

	return paContinue;
}

#endif