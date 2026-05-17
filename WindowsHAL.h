#pragma once
#ifndef WINDOWS_HAL_H
#define WINDOWS_HAL_H

#include "Common.h"
#include "DSP/IHAL.h"
#include "DSP/Math.h"
#include "DSP/Debug.h"
#include "DSP/Memory.h"
#include "WindowsUSBInterface.h"
#include <chrono>
#include <portaudio.h>

#pragma comment(lib, "portaudio.lib")

class WindowsHAL : public IHAL
{
public:
	typedef void (*CrashHandler)(const IHAL* HAL);

public:
	static constexpr uint8 CHANNEL_LEFT = 0;
	static constexpr uint8 CHANNEL_RIGHT = 1;

public:
	WindowsHAL(void* SDRAMAddress = nullptr, uint32 SDRAMSize = 0, CrashHandler CrashHandler = nullptr)
		: m_CrashHandler(CrashHandler),
		m_SDRAMAddress(reinterpret_cast<uint8*>(SDRAMAddress)),
		m_SDRAMSize(SDRAMSize),
		m_LastFreeSDRAMIndex(0),
		m_AudioCallback(nullptr)
	{
		ASSERT(SDRAMSize == 0 || SDRAMAddress != nullptr, "SDRAMAddress cannot be null");
		ASSERT(SDRAMAddress == nullptr || SDRAMSize > 0, "SDRAMSize cannot be zero");
	}

	void Setup(uint8 FrameLength, uint32 SampleRate, bool Boost, bool USBTransmissionMode, bool WaitForDebugger) override
	{
		ASSERT(FrameLength != 0, "Invalid FrameLength %i", FrameLength);

		Pa_Initialize();

		PaStream* stream;
		Pa_OpenDefaultStream(&stream, 2, 2, paFloat32, SampleRate, FrameLength, AudioCallback, this);

		Pa_StartStream(stream);

		if (USBTransmissionMode)
			m_USBInterface.Start();
	}

	void StartAudio(AudioPassthrough Callback) override
	{
		m_AudioCallback = Callback;
	}

	void* Allocate(uint32 Size, bool OnSDRAM = false) override
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

	void Deallocate(void* Memory) override
	{
		if (m_SDRAMAddress != nullptr && m_SDRAMAddress <= Memory)
			return;

		free(Memory);
	}

	bool IsAnAnaloglPin(uint8 Pin) const override
	{
		return true;
	}

	bool IsADigitalPin(uint8 Pin) const override
	{
		return true;
	}

	bool IsAnInputPin(uint8 Pin) const override
	{
		return true;
	}

	bool IsAnOutputPin(uint8 Pin) const override
	{
		return true;
	}

	bool IsInInputMode(uint8 Pin) const override
	{
		return !IsInOutputMode(Pin);
	}

	bool IsInOutputMode(uint8 Pin) const override
	{
		return true;
	}

	bool IsAPWMPin(uint8 Pin) const override
	{
		return true;
	}

	void SetPWMResolution(uint8 Value) override
	{}

	uint8 GetPWMResolution(void) const override
	{
		return 0;
	}

	void SetPinMode(uint8 Pin, PinModes Mode) override
	{}

	float AnalogRead(uint8 Pin) const override
	{
		return 0;
	}

	bool DigitalRead(uint8 Pin) const override
	{
		return false;
	}

	void DigitalWrite(uint8 Pin, bool Value) override
	{}

	void PWMWrite(uint8 Pin, float Value) override
	{}

	uint32 GetTimeFrequency(void) const override
	{
		return 0;
	}

	uint32 GetTimeSinceStartupTicks(void) const override
	{
		return std::chrono::steady_clock::now().time_since_epoch().count();
	}

	uint32 GetTimeSinceStartupMs(void) const override
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
	}

	float GetTimeSinceStartup(void) const override
	{
		return GetTimeSinceStartupMs() / 1000.0;
	}

	void Print(cstr Value) override
	{
		printf(Value);
	}

	void Crash(void) const override
	{
		Delay(1000);

		if (m_CrashHandler != nullptr)
		{
			m_CrashHandler(this);
			return;
		}

		Break();
	}

	void Break(void) const override
	{
		__debugbreak();
	}

	void Reset(void) const override
	{
		exit(0);
	}

	void Delay(uint16 Ms) const override
	{
		_Thrd_sleep_for(Ms);
	}

	IUSBInterface* GetUSBInterface(void) override
	{
		return &m_USBInterface;
	}

	void EraseQSPIData(void) override
	{}

protected:
	void InitializeADC(void)
	{}

	virtual void Update(void)
	{
		m_USBInterface.Update();
	}

private:
	static int AudioCallback(const void* InputBuffer, void* OutputBuffer, unsigned long FramesPerBuffer, const PaStreamCallbackTimeInfo* TimeInfo, PaStreamCallbackFlags StatusFlags, void* UserData)
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

private:
	CrashHandler m_CrashHandler;

	WindowsUSBInterface m_USBInterface;

	uint8* m_SDRAMAddress;
	uint32 m_SDRAMSize;
	uint32 m_LastFreeSDRAMIndex;

	AudioPassthrough m_AudioCallback;
};

#endif