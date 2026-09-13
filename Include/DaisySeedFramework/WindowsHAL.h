#pragma once
#ifndef WINDOWS_HAL_H
#define WINDOWS_HAL_H

#include "Common.h"
#include <DigitalSignalProcessing/IHAL.h>
#include "USB/WindowsUSB.h"
#include <chrono>

struct PaStreamCallbackTimeInfo;

class WindowsHAL : public IHAL
{
public:
	typedef void (*CrashHandler)(const IHAL* HAL);

public:
	static constexpr uint8_t CHANNEL_LEFT = 0;
	static constexpr uint8_t CHANNEL_RIGHT = 1;

public:
	WindowsHAL(void* SDRAMAddress = nullptr, uint32_t SDRAMSize = 0, CrashHandler CrashHandler = nullptr);

	void Setup(uint8_t FrameLength, uint32_t SampleRate, bool Boost) override;

	void StartAudio(AudioPassthrough Callback) override
	{
		m_AudioCallback = Callback;
	}

	void* Allocate(uint32_t Size, bool OnSDRAM = false) override;

	void Deallocate(void* Memory) override;

	bool IsAnAnalogPin(uint8_t Pin) const override
	{
		return true;
	}

	bool IsADigitalPin(uint8_t Pin) const override
	{
		return true;
	}

	bool IsAnInputPin(uint8_t Pin) const override
	{
		return true;
	}

	bool IsAnOutputPin(uint8_t Pin) const override
	{
		return true;
	}

	bool IsInInputMode(uint8_t Pin) const override
	{
		return !IsInOutputMode(Pin);
	}

	bool IsInOutputMode(uint8_t Pin) const override
	{
		return true;
	}

	bool IsAPWMPin(uint8_t Pin) const override
	{
		return true;
	}

	void SetPWMResolution(uint8_t Value) override
	{}

	uint8_t GetPWMResolution(void) const override
	{
		return 0;
	}

	void SetPinMode(uint8_t Pin, PinModes Mode) override
	{}

	float AnalogRead(uint8_t Pin) const override
	{
		return 0;
	}

	bool DigitalRead(uint8_t Pin) const override
	{
		return false;
	}

	void DigitalWrite(uint8_t Pin, bool Value) override
	{}

	void PWMWrite(uint8_t Pin, float Value) override
	{}

	uint32_t GetTimeFrequency(void) const override
	{
		return 0;
	}

	uint32_t GetTimeSinceStartupTicks(void) const override;

	uint32_t GetTimeSinceStartupMs(void) const override;

	float GetTimeSinceStartup(void) const override
	{
		return GetTimeSinceStartupMs() / 1000.0;
	}

	void Print(cstr Value) override;

	bool IsDebuggerPresent(void) const override;

	void Crash(void) const override;

	void Break(void) const override;

	void Reset(bool InfiniteTime = true) const override;

	void Delay(uint16_t Ms) const override;

	IUSB* GetUSB(uint8_t Index) override
	{
		return &m_USB;
	}

	void EraseQSPIData(void) override
	{}

protected:
	void InitializeADC(void)
	{}

	virtual void Update(void);

private:
	static int32_t AudioCallback(const void* InputBuffer, void* OutputBuffer, uint32_t FramesPerBuffer, const PaStreamCallbackTimeInfo* TimeInfo, uint32_t StatusFlags, void* UserData);

private:
	std::chrono::steady_clock::time_point m_StartupTime;

	CrashHandler m_CrashHandler;

	WindowsUSB m_USB;

	uint8_t* m_SDRAMAddress;
	uint32_t m_SDRAMSize;
	uint32_t m_LastFreeSDRAMIndex;

	AudioPassthrough m_AudioCallback;
};

#endif