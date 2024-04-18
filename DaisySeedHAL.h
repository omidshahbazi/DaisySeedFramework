#pragma once
#ifndef DAISY_SEED_HAL_H
#define DAISY_SEED_HAL_H

#include "DSP/IHAL.h"
#include "Common.h"
#include "DSP/Debug.h"
#include <daisy_seed.h>

// const int PWM_FREQUENCY = 240;
// const int PWM_CHANNEL = 1;

class DaisySeedHAL : public IHAL
{
private:
	template <typename T>
	struct PinState
	{
	public:
		T Object;
		uint8 Pin;
		bool Used;
	};

	struct PWMPinState
	{
	public:
		PinState<daisy::GPIO> *State;
		float TargetValue;
		float CurrentValue;
	};

public:
	DaisySeedHAL(daisy::DaisySeed *Hardware)
		: m_Hardware(Hardware),
		  m_AnalogPins{},
		  m_LastFreeAnalogPinIndex(0),
		  m_DigitalPins{},
		  m_PWMPins{},
		  m_LastFreePWMPinIndex(0),
		  m_PWMResolution(0),
		  m_PWMMaxDutyCycle(0)
	{
		// uint8 channelIndex = 0;
		// for (auto &pwmChannel : m_PWMChannels)
		// 	pwmChannel = {false, channelIndex++, 0};

		SetPWMResolution(16);
	}

	void Initialize(void)
	{
		daisy::AdcChannelConfig adcConfigs[ANALOG_PIN_COUNT];
		uint8 index = 0;
		for (const auto &state : m_AnalogPins)
		{
			if (!state.Used)
				continue;

			adcConfigs[index++] = state.Object;
		}

		if (index != 0)
		{
			m_Hardware->adc.Init(adcConfigs, index);
			m_Hardware->adc.Start();
		}
	}

	void Update(void)
	{
		const uint16 SAMPLE_RATE = 1000;
		const float STEP = 120.0F / SAMPLE_RATE;

		for (uint8 i = 0; i < m_LastFreePWMPinIndex; ++i)
		{
			PWMPinState &pwmPin = m_PWMPins[i];

			pwmPin.CurrentValue += STEP;
			if (pwmPin.CurrentValue > 1)
				pwmPin.CurrentValue -= 1;

			pwmPin.State->Object.Write(pwmPin.CurrentValue < pwmPin.TargetValue ? true : false);
		}
	}

	void *Allocate(uint16 Size) override
	{
		return malloc(Size);
	}

	void Deallocate(void *Memory) override
	{
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

	bool IsAPWMPin(uint8 Pin) const override
	{
		return true;
	}

	void SetPWMResolution(uint8 Value) override
	{
		ASSERT(8 <= Value && Value <= 16, "Invalid Value");

		m_PWMResolution = Value;
		m_PWMMaxDutyCycle = (1 << m_PWMResolution) - 1;
	}

	uint8 GetPWMResolution(void) const override
	{
		return m_PWMResolution;
	}

	void SetPinMode(uint8 Pin, PinModes Mode) override
	{
		ASSERT(Mode != PinModes::Input || IsAnInputPin(Pin), "Pin %i is not an input pin", Pin);
		ASSERT(Mode != PinModes::Output || IsAnOutputPin(Pin), "Pin %i is not an output pin", Pin);
		ASSERT(Mode != PinModes::PWM || IsAPWMPin(Pin), "Pin %i is not an PWM pin", Pin);

		daisy::Pin pin = GetPin(Pin);

		if (Mode == PinModes::Input && IsAnAnaloglPin(Pin))
		{
			PinState<daisy::AdcChannelConfig> *state = FindOrGetAnalogPin(Pin);
			state->Object.InitSingle(pin);
			state->Pin = Pin;
			state->Used = true;

			return;
		}

		daisy::GPIO::Config config;
		config.pin = pin;
		config.mode = (Mode == PinModes::Input ? daisy::GPIO::Mode::INPUT : daisy::GPIO::Mode::OUTPUT);
		config.speed = daisy::GPIO::Speed::LOW;

		PinState<daisy::GPIO> &state = m_DigitalPins[GetDigitalPinIndex(Pin)];
		state.Object.Init(config);
		state.Pin = Pin;
		state.Used = true;

		if (Mode == PinModes::PWM)
		{
			PWMPinState *pwmPinState = FindOrGetPWMPin(Pin);
			pwmPinState->State = &state;
			pwmPinState->CurrentValue = 0;
			pwmPinState->TargetValue = 0;
		}
	}

	float AnalogRead(uint8 Pin) const override
	{
		ASSERT(IsAnAnaloglPin(Pin), "Pin %i is not an analog pin", Pin);

		return m_Hardware->adc.GetFloat(GetAnalogPinIndex(Pin));
	}

	bool DigitalRead(uint8 Pin) const override
	{
		ASSERT(IsADigitalPin(Pin), "Pin %i is not an digital pin", Pin);

		PinState<daisy::GPIO> &state = const_cast<PinState<daisy::GPIO> &>(m_DigitalPins[GetDigitalPinIndex(Pin)]);
		return state.Object.Read();
	}

	void DigitalWrite(uint8 Pin, bool Value) override
	{
		ASSERT(IsADigitalPin(Pin), "Pin %i is not an digital pin", Pin);

		PinState<daisy::GPIO> &state = const_cast<PinState<daisy::GPIO> &>(m_DigitalPins[GetDigitalPinIndex(Pin)]);
		state.Object.Write(Value);
	}

	void PWMWrite(uint8 Pin, float Value) override
	{
		ASSERT(0 <= Value && Value <= 1, "Invalid Value");

		FindOrGetPWMPin(Pin)->TargetValue = Math::Cube(Value);
	}

	float GetTimeSinceStartup(void) const override
	{
		return daisy::System::GetNow() / 1000.0F;
	}

	void Print(const char *Value) const override
	{
		daisy::DaisySeed::Print(Value);
	}

	void Break(void) const override
	{
		asm("bkpt 255");

		while (1)
		{
			Delay(1);
		}
	}

	void Delay(uint16 Ms) const override
	{
		daisy::System::Delay(Ms);
	}

private:
	daisy::Pin GetPin(uint8 Pin) const
	{
		switch (Pin)
		{
		case 0:
			return daisy::seed::D0;
		case 1:
			return daisy::seed::D1;
		case 2:
			return daisy::seed::D2;
		case 3:
			return daisy::seed::D3;
		case 4:
			return daisy::seed::D4;
		case 5:
			return daisy::seed::D5;
		case 6:
			return daisy::seed::D6;
		case 7:
			return daisy::seed::D7;
		case 8:
			return daisy::seed::D8;
		case 9:
			return daisy::seed::D9;
		case 10:
			return daisy::seed::D10;
		case 11:
			return daisy::seed::D11;
		case 12:
			return daisy::seed::D12;
		case 13:
			return daisy::seed::D13;
		case 14:
			return daisy::seed::D14;
		case 15:
			return daisy::seed::D15;
		case 16:
			return daisy::seed::D16;
		case 17:
			return daisy::seed::D17;
		case 18:
			return daisy::seed::D18;
		case 19:
			return daisy::seed::D19;
		case 20:
			return daisy::seed::D20;
		case 21:
			return daisy::seed::D21;
		case 22:
			return daisy::seed::D22;
		case 23:
			return daisy::seed::D23;
		case 24:
			return daisy::seed::D24;
		case 25:
			return daisy::seed::D25;
		case 26:
			return daisy::seed::D26;
		case 27:
			return daisy::seed::D27;
		case 28:
			return daisy::seed::D28;
		case 29:
			return daisy::seed::D29;
		case 30:
			return daisy::seed::D30;
		case 31:
			return daisy::seed::D31;
		}
	}

	uint8 GetAnalogPinIndex(uint8 Pin) const
	{
		uint8 index = 0;
		for (auto &state : m_AnalogPins)
		{
			if (state.Pin != Pin)
			{
				++index;
				continue;
			}

			return index;
		}

		ASSERT(false, "Couldn't find the state for pin %i", Pin);
	}

	PinState<daisy::AdcChannelConfig> *FindOrGetAnalogPin(uint8 Pin)
	{
		for (auto &state : m_AnalogPins)
		{
			if (state.Pin != Pin)
				continue;

			return &state;
		}

		ASSERT(m_LastFreeAnalogPinIndex < ANALOG_PIN_COUNT, "Out of free Analog pins");

		return &m_AnalogPins[m_LastFreeAnalogPinIndex++];
	}

	uint8 GetDigitalPinIndex(uint8 Pin) const
	{
		ASSERT(IsADigitalPin(Pin), "Pin %i is not an digital pin", Pin);

		return (uint8)Pin - (uint8)GPIOPins::Pin0;
	}

	PWMPinState *FindOrGetPWMPin(uint8 Pin)
	{
		for (auto &state : m_PWMPins)
		{
			if (state.State == nullptr || state.State->Pin != Pin)
				continue;

			return &state;
		}

		ASSERT(m_LastFreePWMPinIndex < (uint8)GPIOPins::COUNT, "Out of free PWM pin states");

		return &m_PWMPins[m_LastFreePWMPinIndex++];
	}

private:
	daisy::DaisySeed *m_Hardware;
	PinState<daisy::AdcChannelConfig> m_AnalogPins[ANALOG_PIN_COUNT];
	uint8 m_LastFreeAnalogPinIndex;

	PinState<daisy::GPIO> m_DigitalPins[(uint8)GPIOPins::COUNT];

	PWMPinState m_PWMPins[(uint8)GPIOPins::COUNT];
	uint8 m_LastFreePWMPinIndex;

	uint8 m_PWMResolution;
	uint32 m_PWMMaxDutyCycle;
};

#endif