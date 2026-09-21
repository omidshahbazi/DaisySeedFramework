#pragma once
#ifndef DAISY_SEED_HAL_H
#define DAISY_SEED_HAL_H

#include "Common.h"
#include "DaisyInclude.h"
#include "USB/DaisyUSB.h"
#include "DaisyFirmware.h"
#include <DigitalSignalProcessing/IHAL.h>
#include <DigitalSignalProcessing/Allocator.h>

class DaisySeedHAL : public IHAL
{
public:
	static constexpr uint8_t CHANNEL_LEFT = 0;
	static constexpr uint8_t CHANNEL_RIGHT = 1;

private:
	template <typename T>
	struct PinState
	{
	public:
		T Object;
		uint8_t Pin;
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
	DaisySeedHAL(void* SDRAMAddress = nullptr, uint32_t SDRAMSize = 0);

	void Setup(uint8_t FrameLength, uint32_t SampleRate, bool Boost) override;

	void StartAudio(AudioPassthrough Callback) override;

	void* Allocate(uint32_t Size, bool OnSDRAM = false) override;

	void Deallocate(void* Memory) override;

	bool IsAnAnalogPin(uint8_t Pin) const override;

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

	bool IsInOutputMode(uint8_t Pin) const override;

	bool IsAPWMPin(uint8_t Pin) const override
	{
		return true;
	}

	void SetPWMResolution(uint8_t Value) override;

	uint8_t GetPWMResolution(void) const override
	{
		return m_PWMResolution;
	}

	void SetPinMode(uint8_t Pin, PinModes Mode) override;

	float AnalogRead(uint8_t Pin) const override;

	bool DigitalRead(uint8_t Pin) const override;

	void DigitalWrite(uint8_t Pin, bool Value) override;

	void PWMWrite(uint8_t Pin, float Value) override;

	uint32_t GetTimeFrequency(void) const override;

	uint32_t GetTimeSinceStartupTicks(void) const override;

	uint32_t GetTimeSinceStartupMs(void) const override;

	float GetTimeSinceStartup(void) const override
	{
		return GetTimeSinceStartupMs() / 1000.0;
	}

	// Bootloader version has to be in sync with the libDaisy, so if you see malfunction here, update either of them
	// https://flash.daisy.audio/
	void Reset(bool InfiniteTime = true) const override;

	void Delay(uint16_t Ms) const override;

	IUSB* GetUSB(uint8_t Index = 0) override
	{
		if (Index == 0)
			return &m_HighSpeedUSB;

		return &m_FullSpeedUSB;
	}

	IFirmware* GetFirmware(void) override
	{
		return &m_Firmware;
	}

	void EraseUserData(void) override;

	daisy::QSPIHandle& GetQSPI(void);

public:
	static daisy::Pin GetPin(uint8_t Pin);

protected:
	void InitializeADC(void);

	virtual void Update(void);

private:
	uint8_t GetAnalogPinIndex(uint8_t Pin) const;

	PinState<daisy::AdcChannelConfig>* FindAnalogPin(uint8_t Pin);

	const PinState<daisy::AdcChannelConfig>* FindAnalogPin(uint8_t Pin) const;

	PinState<daisy::AdcChannelConfig>* FindOrGetNewAnalogPin(uint8_t Pin);

	PinState<daisy::GPIO>& GetDigitalPinState(uint8_t Pin);

	const PinState<daisy::GPIO>& GetDigitalPinState(uint8_t Pin) const;

	uint8_t GetDigitalPinIndex(uint8_t Pin) const;

	PWMPinState* FindOrGetPWMPin(uint8_t Pin);

private:
	daisy::DaisySeed m_Hardware;

	Allocator<uint8_t, 16, false> m_Allocator;

	DaisyUSB m_FullSpeedUSB;
	DaisyUSB m_HighSpeedUSB;

	DaisyFirmware m_Firmware;

	uint8_t* m_SDRAMAddress;
	uint32_t m_SDRAMSize;
	uint32_t m_LastFreeSDRAMIndex;

	PinState<daisy::AdcChannelConfig> m_AnalogPins[(uint8_t)AnalogPins::COUNT];
	uint8_t m_LastFreeAnalogPinIndex;

	PinState<daisy::GPIO> m_DigitalPins[(uint8_t)GPIOPins::COUNT];

	PWMPinState m_PWMPins[(uint8_t)GPIOPins::COUNT];
	uint8_t m_LastFreePWMPinIndex;

	uint8_t m_PWMResolution;
	uint32_t m_PWMMaxDutyCycle;
};

#endif