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

class WindowsHAL : public IHAL
{
public:
	typedef void (*CrashHandler)(const IHAL* HAL);

public:
	static constexpr uint8 CHANNEL_LEFT = 0;
	static constexpr uint8 CHANNEL_RIGHT = 1;

private:
	//template <typename T>
	//struct PinState
	//{
	//public:
	//	T Object;
	//	uint8 Pin;
	//	PinModes Mode;
	//	bool Used;
	//};

	//struct PWMPinState
	//{
	//public:
	//	PinState<daisy::GPIO> *State;
	//	float TargetValue;
	//	float CurrentValue;
	//};

public:
	WindowsHAL(void* SDRAMAddress = nullptr, uint32 SDRAMSize = 0, CrashHandler CrashHandler = nullptr)
		: m_CrashHandler(CrashHandler),
		m_SDRAMAddress(reinterpret_cast<uint8*>(SDRAMAddress)),
		m_SDRAMSize(SDRAMSize),
		m_LastFreeSDRAMIndex(0)
		/*,
		  m_AnalogPins{},
		  m_LastFreeAnalogPinIndex(0),
		  m_DigitalPins{},
		  m_PWMPins{},
		  m_LastFreePWMPinIndex(0),
		  m_PWMResolution(0),
		  m_PWMMaxDutyCycle(0)*/
	{
		ASSERT(SDRAMSize == 0 || SDRAMAddress != nullptr, "SDRAMAddress cannot be null");
		ASSERT(SDRAMAddress == nullptr || SDRAMSize > 0, "SDRAMSize cannot be zero");
	}

	void Setup(uint8 FrameLength, uint32 SampleRate, bool Boost, bool USBTransmissionMode, bool WaitForDebugger) override
	{
		//ASSERT(FrameLength != 0, "Invalid FrameLength %i", FrameLength);

		//m_Hardware->Init(Boost);
		//m_Hardware->SetAudioBlockSize(FrameLength);

		//m_Hardware->StartLog(WaitForDebugger);

		//if (USBTransmissionMode)
		//	m_USBInterface.Start();

		//daisy::SaiHandle::Config::SampleRate daisySampleRate;
		//switch (SampleRate)
		//{
		//case SAMPLE_RATE_8000:
		//	daisySampleRate = daisy::SaiHandle::Config::SampleRate::SAI_8KHZ;
		//	break;

		//case SAMPLE_RATE_16000:
		//	daisySampleRate = daisy::SaiHandle::Config::SampleRate::SAI_16KHZ;
		//	break;

		//case SAMPLE_RATE_32000:
		//	daisySampleRate = daisy::SaiHandle::Config::SampleRate::SAI_32KHZ;
		//	break;

		//case SAMPLE_RATE_48000:
		//	daisySampleRate = daisy::SaiHandle::Config::SampleRate::SAI_48KHZ;
		//	break;

		//case SAMPLE_RATE_96000:
		//	daisySampleRate = daisy::SaiHandle::Config::SampleRate::SAI_96KHZ;
		//	break;

		//default:
		//	ASSERT(false, "No sutaible sample rate for %i found in the daisy", SampleRate);
		//}

		//m_Hardware->SetAudioSampleRate(daisySampleRate);
	}

	void StartAudio(AudioPassthrough Callback) override
	{
		//m_Hardware->StartAudio(Callback);
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
		switch (Pin)
		{
		case (uint8)GPIOPins::Pin15:
		case (uint8)GPIOPins::Pin16:
		case (uint8)GPIOPins::Pin17:
		case (uint8)GPIOPins::Pin18:
		case (uint8)GPIOPins::Pin19:
		case (uint8)GPIOPins::Pin20:
		case (uint8)GPIOPins::Pin21:
		case (uint8)GPIOPins::Pin22:
		case (uint8)GPIOPins::Pin23:
		case (uint8)GPIOPins::Pin24:
		case (uint8)GPIOPins::Pin25:
		case (uint8)GPIOPins::Pin28:
			return true;

		default:
			return false;
		}
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

	// Bootloader version has to be in sync with the libDaisy, so if you see mal-function here, update either of them
	// https://flash.daisy.audio/
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

	//public:
	//	static daisy::Pin GetPin(uint8 Pin)
	//	{
	//		switch (Pin)
	//		{
	//		case 0:
	//			return daisy::seed::D0;
	//		case 1:
	//			return daisy::seed::D1;
	//		case 2:
	//			return daisy::seed::D2;
	//		case 3:
	//			return daisy::seed::D3;
	//		case 4:
	//			return daisy::seed::D4;
	//		case 5:
	//			return daisy::seed::D5;
	//		case 6:
	//			return daisy::seed::D6;
	//		case 7:
	//			return daisy::seed::D7;
	//		case 8:
	//			return daisy::seed::D8;
	//		case 9:
	//			return daisy::seed::D9;
	//		case 10:
	//			return daisy::seed::D10;
	//		case 11:
	//			return daisy::seed::D11;
	//		case 12:
	//			return daisy::seed::D12;
	//		case 13:
	//			return daisy::seed::D13;
	//		case 14:
	//			return daisy::seed::D14;
	//		case 15:
	//			return daisy::seed::D15;
	//		case 16:
	//			return daisy::seed::D16;
	//		case 17:
	//			return daisy::seed::D17;
	//		case 18:
	//			return daisy::seed::D18;
	//		case 19:
	//			return daisy::seed::D19;
	//		case 20:
	//			return daisy::seed::D20;
	//		case 21:
	//			return daisy::seed::D21;
	//		case 22:
	//			return daisy::seed::D22;
	//		case 23:
	//			return daisy::seed::D23;
	//		case 24:
	//			return daisy::seed::D24;
	//		case 25:
	//			return daisy::seed::D25;
	//		case 26:
	//			return daisy::seed::D26;
	//		case 27:
	//			return daisy::seed::D27;
	//		case 28:
	//			return daisy::seed::D28;
	//		case 29:
	//			return daisy::seed::D29;
	//		case 30:
	//			return daisy::seed::D30;
	//		case 31:
	//			return daisy::seed::D31;
	//		}
	//
	//		ASSERT(false, "Invalid Pin %i", Pin);
	//	}

protected:
	void InitializeADC(void)
	{
		//daisy::AdcChannelConfig adcConfigs[ANALOG_PIN_COUNT];
		//uint8 index = 0;
		//for (const auto &state : m_AnalogPins)
		//{
		//	if (!state.Used)
		//		continue;

		//	adcConfigs[index++] = state.Object;
		//}

		//if (index != 0)
		//{
		//	m_Hardware->adc.Init(adcConfigs, index);
		//	m_Hardware->adc.Start();
		//}
	}

	virtual void Update(void)
	{
		m_USBInterface.Update();

		//const uint16 SAMPLE_RATE = 1000;
		//const float STEP = 120.0F / SAMPLE_RATE;

		//for (uint8 i = 0; i < m_LastFreePWMPinIndex; ++i)
		//{
		//	PWMPinState &pwmPin = m_PWMPins[i];

		//	pwmPin.CurrentValue += STEP;
		//	if (pwmPin.CurrentValue > 1)
		//		pwmPin.CurrentValue -= 1;

		//	pwmPin.State->Object.Write(pwmPin.CurrentValue < pwmPin.TargetValue ? true : false);
		//}
	}

	//private:
	//	uint8 GetAnalogPinIndex(uint8 Pin) const
	//	{
	//		uint8 index = 0;
	//		for (auto &state : m_AnalogPins)
	//		{
	//			if (state.Pin != Pin)
	//			{
	//				++index;
	//				continue;
	//			}
	//
	//			return index;
	//		}
	//
	//		ASSERT(false, "Couldn't find the state for pin %i", Pin);
	//	}
	//
	//	PinState<daisy::AdcChannelConfig> *FindAnalogPin(uint8 Pin)
	//	{
	//		for (auto &state : m_AnalogPins)
	//		{
	//			if (state.Pin != Pin)
	//				continue;
	//
	//			return &state;
	//		}
	//
	//		return nullptr;
	//	}
	//
	//	const PinState<daisy::AdcChannelConfig> *FindAnalogPin(uint8 Pin) const
	//	{
	//		for (auto &state : m_AnalogPins)
	//		{
	//			if (state.Pin != Pin)
	//				continue;
	//
	//			return &state;
	//		}
	//
	//		return nullptr;
	//	}
	//
	//	PinState<daisy::AdcChannelConfig> *FindOrGetNewAnalogPin(uint8 Pin)
	//	{
	//		PinState<daisy::AdcChannelConfig> *state = FindAnalogPin(Pin);
	//		if (state != nullptr)
	//			return state;
	//
	//		ASSERT(m_LastFreeAnalogPinIndex < ANALOG_PIN_COUNT, "Out of free Analog pins");
	//
	//		return &m_AnalogPins[m_LastFreeAnalogPinIndex++];
	//	}
	//
	//	PinState<daisy::GPIO> &GetDigitalPinState(uint8 Pin)
	//	{
	//		return m_DigitalPins[GetDigitalPinIndex(Pin)];
	//	}
	//
	//	const PinState<daisy::GPIO> &GetDigitalPinState(uint8 Pin) const
	//	{
	//		return m_DigitalPins[GetDigitalPinIndex(Pin)];
	//	}
	//
	//	uint8 GetDigitalPinIndex(uint8 Pin) const
	//	{
	//		ASSERT(IsADigitalPin(Pin), "Pin %i is not an digital pin", Pin);
	//
	//		return (uint8)Pin - (uint8)GPIOPins::Pin0;
	//	}
	//
	//	PWMPinState *FindOrGetPWMPin(uint8 Pin)
	//	{
	//		for (auto &state : m_PWMPins)
	//		{
	//			if (state.State == nullptr || state.State->Pin != Pin)
	//				continue;
	//
	//			return &state;
	//		}
	//
	//		ASSERT(m_LastFreePWMPinIndex < (uint8)GPIOPins::COUNT, "Out of free PWM pin states");
	//
	//		return &m_PWMPins[m_LastFreePWMPinIndex++];
	//	}

private:
	CrashHandler m_CrashHandler;

	WindowsUSBInterface m_USBInterface;

	uint8* m_SDRAMAddress;
	uint32 m_SDRAMSize;
	uint32 m_LastFreeSDRAMIndex;

	//PinState<daisy::AdcChannelConfig> m_AnalogPins[ANALOG_PIN_COUNT];
	//uint8 m_LastFreeAnalogPinIndex;

	//PinState<daisy::GPIO> m_DigitalPins[(uint8)GPIOPins::COUNT];

	//PWMPinState m_PWMPins[(uint8)GPIOPins::COUNT];
	//uint8 m_LastFreePWMPinIndex;

	//uint8 m_PWMResolution;
	//uint32 m_PWMMaxDutyCycle;
};

#endif