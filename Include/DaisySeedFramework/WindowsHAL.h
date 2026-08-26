#pragma once
#ifndef WINDOWS_HAL_H
#define WINDOWS_HAL_H

#include "Common.h"
#include <DigitalSignalProcessing/IHAL.h>
#include "WindowsUSBInterface.h"
#include <chrono>

struct PaStreamCallbackTimeInfo;

class WindowsHAL : public IHAL
{
public:
	typedef void (*CrashHandler)(const IHAL* HAL);

public:
	static constexpr uint8 CHANNEL_LEFT = 0;
	static constexpr uint8 CHANNEL_RIGHT = 1;

public:
	WindowsHAL(void* SDRAMAddress = nullptr, uint32 SDRAMSize = 0, CrashHandler CrashHandler = nullptr);

	void Setup(uint8 FrameLength, uint32 SampleRate, bool Boost) override;

	void StartAudio(AudioPassthrough Callback) override
	{
		m_AudioCallback = Callback;
	}

	void* Allocate(uint32 Size, bool OnSDRAM = false) override;

	void Deallocate(void* Memory) override;

	bool IsAnAnalogPin(uint8 Pin) const override
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

	uint32 GetTimeSinceStartupTicks(void) const override;

	uint32 GetTimeSinceStartupMs(void) const override;

	float GetTimeSinceStartup(void) const override
	{
		return GetTimeSinceStartupMs() / 1000.0;
	}

	void Print(cstr Value) override;

	bool IsDebuggerPresent(void) const override;

	void Crash(void) const override;

	void Break(void) const override;

	void Reset(bool InfiniteTime = true) const override;

	void Delay(uint16 Ms) const override;

	IUSBInterface* GetUSBInterface(void) override
	{
		return &m_USBInterface;
	}

	void EraseQSPIData(void) override
	{}

protected:
	void InitializeADC(void)
	{}

	virtual void Update(void);

private:
	static int AudioCallback(const void* InputBuffer, void* OutputBuffer, uint32 FramesPerBuffer, const PaStreamCallbackTimeInfo* TimeInfo, uint32 StatusFlags, void* UserData);

private:
	std::chrono::steady_clock::time_point m_StartupTime;

	CrashHandler m_CrashHandler;

	WindowsUSBInterface m_USBInterface;

	uint8* m_SDRAMAddress;
	uint32 m_SDRAMSize;
	uint32 m_LastFreeSDRAMIndex;

	AudioPassthrough m_AudioCallback;
};

#endif