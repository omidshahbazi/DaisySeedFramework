#pragma once
#ifndef DAISY_SEED_HAL_H
#define DAISY_SEED_HAL_H

#include "Common.h"
#include "DaisyInclude.h"
#include "USB/DaisyUSB.h"
#include <DigitalSignalProcessing/IHAL.h>

class DaisySeedHAL : public IHAL
{
public:
	typedef void (*CrashHandler)(const IHAL* HAL);

public:
	static constexpr uint8 CHANNEL_LEFT = 0;
	static constexpr uint8 CHANNEL_RIGHT = 1;

private:
	template <typename T>
	struct PinState
	{
	public:
		T Object;
		uint8 Pin;
		PinModes Mode;
		bool Used;
	};

	struct PWMPinState
	{
	public:
		PinState<daisy::GPIO>* State;
		float TargetValue;
		float CurrentValue;
	};

public:
	DaisySeedHAL(void* SDRAMAddress = nullptr, uint32 SDRAMSize = 0, CrashHandler CrashHandler = nullptr);

	void Setup(uint8 FrameLength, uint32 SampleRate, bool Boost) override;

	void StartAudio(AudioPassthrough Callback) override;

	void* Allocate(uint32 Size, bool OnSDRAM = false) override;

	void Deallocate(void* Memory) override;

	bool IsAnAnalogPin(uint8 Pin) const override;

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

	bool IsInOutputMode(uint8 Pin) const override;

	bool IsAPWMPin(uint8 Pin) const override
	{
		return true;
	}

	void SetPWMResolution(uint8 Value) override;

	uint8 GetPWMResolution(void) const override
	{
		return m_PWMResolution;
	}

	void SetPinMode(uint8 Pin, PinModes Mode) override;

	float AnalogRead(uint8 Pin) const override;

	bool DigitalRead(uint8 Pin) const override;

	void DigitalWrite(uint8 Pin, bool Value) override;

	void PWMWrite(uint8 Pin, float Value) override;

	uint32 GetTimeFrequency(void) const override;

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

	// Bootloader version has to be in sync with the libDaisy, so if you see malfunction here, update either of them
	// https://flash.daisy.audio/
	void Reset(bool InfiniteTime = true) const override;

	void Delay(uint16 Ms) const override;

	IUSB* GetUSB(uint8 Index = 0) override
	{
		if (Index == 0)
			return &m_HighSpeedUSB;

		return &m_FullSpeedUSB;
	}

	void EraseQSPIData(void) override;

	daisy::QSPIHandle& GetQSPI(void);

public:
	static daisy::Pin GetPin(uint8 Pin);

protected:
	void InitializeADC(void);

	virtual void Update(void);

private:
	uint8 GetAnalogPinIndex(uint8 Pin) const;

	PinState<daisy::AdcChannelConfig>* FindAnalogPin(uint8 Pin);

	const PinState<daisy::AdcChannelConfig>* FindAnalogPin(uint8 Pin) const;

	PinState<daisy::AdcChannelConfig>* FindOrGetNewAnalogPin(uint8 Pin);

	PinState<daisy::GPIO>& GetDigitalPinState(uint8 Pin);

	const PinState<daisy::GPIO>& GetDigitalPinState(uint8 Pin) const;

	uint8 GetDigitalPinIndex(uint8 Pin) const;

	PWMPinState* FindOrGetPWMPin(uint8 Pin);

private:
	daisy::DaisySeed m_Hardware;
	CrashHandler m_CrashHandler;

	DaisyUSB m_FullSpeedUSB;
	DaisyUSB m_HighSpeedUSB;

	uint8* m_SDRAMAddress;
	uint32 m_SDRAMSize;
	uint32 m_LastFreeSDRAMIndex;

	PinState<daisy::AdcChannelConfig> m_AnalogPins[(uint8)AnalogPins::COUNT];
	uint8 m_LastFreeAnalogPinIndex;

	PinState<daisy::GPIO> m_DigitalPins[(uint8)GPIOPins::COUNT];

	PWMPinState m_PWMPins[(uint8)GPIOPins::COUNT];
	uint8 m_LastFreePWMPinIndex;

	uint8 m_PWMResolution;
	uint32 m_PWMMaxDutyCycle;
};

#endif