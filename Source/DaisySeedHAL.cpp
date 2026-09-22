#ifdef ON_HARDWARE

#include "DaisySeedFramework/DaisySeedHAL.h"
#include <DigitalSignalProcessing/Math.h>
#include <DigitalSignalProcessing/Debug.h>

DaisySeedHAL::DaisySeedHAL(void* SDRAMAddress, uint32_t SDRAMSize)
	: m_Firmware(this),
	m_FullSpeedUSB(Peripherals::FullSpeed),
	m_HighSpeedUSB(Peripherals::HighSpeed),
	m_Allocator(SDRAMAddress, SDRAMSize),
	m_LastFreeSDRAMIndex(0),
	m_AnalogPins{},
	m_LastFreeAnalogPinIndex(0),
	m_DigitalPins{},
	m_PWMPins{},
	m_LastFreePWMPinIndex(0),
	m_PWMResolution(0),
	m_PWMMaxDutyCycle(0)
{

	SetPWMResolution(16);
}

void DaisySeedHAL::Setup(uint8_t FrameLength, uint32_t SampleRate, bool Boost)
{
	ASSERT(FrameLength != 0, "Invalid FrameLength %i", FrameLength);

	m_Hardware.Init(Boost);
	m_Hardware.SetAudioBlockSize(FrameLength);

	m_Hardware.StartLog();

	daisy::SaiHandle::Config::SampleRate daisySampleRate;
	switch (SampleRate)
	{
	case SAMPLE_RATE_8000:
		daisySampleRate = daisy::SaiHandle::Config::SampleRate::SAI_8KHZ;
		break;

	case SAMPLE_RATE_16000:
		daisySampleRate = daisy::SaiHandle::Config::SampleRate::SAI_16KHZ;
		break;

	case SAMPLE_RATE_32000:
		daisySampleRate = daisy::SaiHandle::Config::SampleRate::SAI_32KHZ;
		break;

	case SAMPLE_RATE_48000:
		daisySampleRate = daisy::SaiHandle::Config::SampleRate::SAI_48KHZ;
		break;

	case SAMPLE_RATE_96000:
		daisySampleRate = daisy::SaiHandle::Config::SampleRate::SAI_96KHZ;
		break;

	default:
		NOT_IMPLEMENTED();
	}

	m_Hardware.SetAudioSampleRate(daisySampleRate);
}

void DaisySeedHAL::StartAudio(AudioPassthrough Callback)
{
	m_Hardware.StartAudio((daisy::AudioHandle::AudioCallback)Callback);
}

void* DaisySeedHAL::Allocate(uint32_t Size, bool OnSDRAM)
{
	if (OnSDRAM)
		return m_Allocator.Allocate(Size);

	return malloc(Size);
}

void DaisySeedHAL::Deallocate(void* Memory)
{
	if (m_Allocator.Contains(Memory))
		return;

	free(Memory);
}

bool DaisySeedHAL::IsAnAnalogPin(uint8_t Pin) const
{
	switch (Pin)
	{
	case (uint8_t)GPIOPins::Pin15:
	case (uint8_t)GPIOPins::Pin16:
	case (uint8_t)GPIOPins::Pin17:
	case (uint8_t)GPIOPins::Pin18:
	case (uint8_t)GPIOPins::Pin19:
	case (uint8_t)GPIOPins::Pin20:
	case (uint8_t)GPIOPins::Pin21:
	case (uint8_t)GPIOPins::Pin22:
	case (uint8_t)GPIOPins::Pin23:
	case (uint8_t)GPIOPins::Pin24:
	case (uint8_t)GPIOPins::Pin25:
	case (uint8_t)GPIOPins::Pin28:
		return true;

	default:
		return false;
	}
}

bool DaisySeedHAL::IsInOutputMode(uint8_t Pin) const
{
	const PinState<daisy::AdcChannelConfig>* analogPinState = FindAnalogPin(Pin);
	if (analogPinState != nullptr)
		return false;

	const PinState<daisy::GPIO>& digitalPinState = GetDigitalPinState(Pin);

	return (digitalPinState.Mode == PinModes::DigitalOutput || digitalPinState.Mode == PinModes::PWM);
}

void DaisySeedHAL::SetPWMResolution(uint8_t Value)
{
	ASSERT(8 <= Value && Value <= 16, "Invalid Value %f", Value);

	m_PWMResolution = Value;
	m_PWMMaxDutyCycle = (1 << m_PWMResolution) - 1;
}

void DaisySeedHAL::SetPinMode(uint8_t Pin, PinModes Mode)
{
	ASSERT((Mode != PinModes::AnalogInput && Mode != PinModes::DigitalInput) || IsAnInputPin(Pin), "Pin %i is not an input pin", Pin);
	ASSERT(Mode != PinModes::DigitalOutput || IsAnOutputPin(Pin), "Pin %i is not an output pin", Pin);
	ASSERT(Mode != PinModes::PWM || IsAPWMPin(Pin), "Pin %i is not an PWM pin", Pin);

	daisy::Pin pin = GetPin(Pin);

	if (Mode == PinModes::AnalogInput && IsAnAnalogPin(Pin))
	{
		PinState<daisy::AdcChannelConfig>* state = FindOrGetNewAnalogPin(Pin);
		state->Object.InitSingle(pin);
		state->Pin = Pin;
		state->Used = true;
		state->Mode = Mode;

		return;
	}

	daisy::GPIO::Config config;
	config.pin = pin;
	config.mode = (Mode == PinModes::DigitalInput ? daisy::GPIO::Mode::INPUT : daisy::GPIO::Mode::OUTPUT);
	config.speed = daisy::GPIO::Speed::LOW;
	config.pull = daisy::GPIO::Pull::PULLUP;

	PinState<daisy::GPIO>& state = GetDigitalPinState(Pin);
	state.Object.Init(config);
	state.Pin = Pin;
	state.Used = true;
	state.Mode = Mode;

	if (Mode == PinModes::PWM)
	{
		PWMPinState* pwmPinState = FindOrGetPWMPin(Pin);
		pwmPinState->State = &state;
		pwmPinState->CurrentValue = 0;
		pwmPinState->TargetValue = 0;
	}
}

float DaisySeedHAL::AnalogRead(uint8_t Pin) const
{
	ASSERT(IsAnAnalogPin(Pin), "Pin %i is not an analog pin", Pin);

	return m_Hardware.adc.GetFloat(GetAnalogPinIndex(Pin));
}

bool DaisySeedHAL::DigitalRead(uint8_t Pin) const
{
	ASSERT(IsADigitalPin(Pin), "Pin %i is not an digital pin", Pin);
	ASSERT(IsInInputMode(Pin), "Pin %i is not in input mode", Pin);

	PinState<daisy::GPIO>& state = const_cast<PinState<daisy::GPIO> &>(GetDigitalPinState(Pin));
	return !state.Object.Read();
}

void DaisySeedHAL::DigitalWrite(uint8_t Pin, bool Value)
{
	ASSERT(IsADigitalPin(Pin), "Pin %i is not an digital pin", Pin);
	ASSERT(IsInOutputMode(Pin), "Pin %i is not in output mode", Pin);

	PinState<daisy::GPIO>& state = const_cast<PinState<daisy::GPIO> &>(GetDigitalPinState(Pin));
	state.Object.Write(Value);
}

void DaisySeedHAL::PWMWrite(uint8_t Pin, float Value)
{
	ASSERT(0 <= Value && Value <= 1, "Invalid Value %f", Value);
	ASSERT(IsInOutputMode(Pin), "Pin %i is not in output mode", Pin);

	FindOrGetPWMPin(Pin)->TargetValue = Math::Cube(Value);
}

uint32_t DaisySeedHAL::GetTimeFrequency(void) const
{
	return daisy::System::GetTickFreq();
}

uint32_t DaisySeedHAL::GetTimeSinceStartupTicks(void) const
{
	return daisy::System::GetTick();
}

uint32_t DaisySeedHAL::GetTimeSinceStartupMs(void) const
{
	return daisy::System::GetNow();
}

// Bootloader version has to be in sync with the libDaisy, so if you see mal-function here, update either of them
// https://flash.daisy.audio/
void DaisySeedHAL::Reset(bool InfiniteTime) const
{
	daisy::System::ResetToBootloader(InfiniteTime ? daisy::System::BootloaderMode::DAISY_INFINITE_TIMEOUT : daisy::System::BootloaderMode::DAISY_SKIP_TIMEOUT);
}

void DaisySeedHAL::Delay(uint16_t Ms) const
{
	daisy::System::Delay(Ms);
}

void DaisySeedHAL::EraseUserData(void)
{
	m_Hardware.qspi.Erase(QSPI_USER_DATA_START_ADDRESS, QSPI_END_ADDRESS);
}

daisy::QSPIHandle& DaisySeedHAL::GetQSPI(void)
{
	return m_Hardware.qspi;
}

daisy::Pin DaisySeedHAL::GetPin(uint8_t Pin)
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

	default:
		NOT_IMPLEMENTED();
	}
}

void DaisySeedHAL::InitializeADC(void)
{
	daisy::AdcChannelConfig adcConfigs[(uint8_t)AnalogPins::COUNT];
	uint8_t index = 0;
	for (const auto& state : m_AnalogPins)
	{
		if (!state.Used)
			continue;

		adcConfigs[index++] = state.Object;
	}

	if (index != 0)
	{
		m_Hardware.adc.Init(adcConfigs, index);
		m_Hardware.adc.Start();
	}
}

void DaisySeedHAL::Update(void)
{
	//m_USBInterface.Update();

	const uint16_t SAMPLE_RATE = 1000;
	const float STEP = 120.0F / SAMPLE_RATE;

	for (uint8_t i = 0; i < m_LastFreePWMPinIndex; ++i)
	{
		PWMPinState& pwmPin = m_PWMPins[i];

		pwmPin.CurrentValue += STEP;
		if (pwmPin.CurrentValue > 1)
			pwmPin.CurrentValue -= 1;

		pwmPin.State->Object.Write(pwmPin.CurrentValue < pwmPin.TargetValue ? true : false);
	}

	m_FullSpeedUSB.Update();
	m_HighSpeedUSB.Update();
}

uint8_t DaisySeedHAL::GetAnalogPinIndex(uint8_t Pin) const
{
	uint8_t index = 0;
	for (auto& state : m_AnalogPins)
	{
		if (state.Pin != Pin)
		{
			++index;
			continue;
		}

		return index;
	}

	BREAK("Couldn't find the state for pin %i", Pin);
}

DaisySeedHAL::PinState<daisy::AdcChannelConfig>* DaisySeedHAL::FindAnalogPin(uint8_t Pin)
{
	for (auto& state : m_AnalogPins)
	{
		if (state.Pin != Pin)
			continue;

		return &state;
	}

	return nullptr;
}

const DaisySeedHAL::PinState<daisy::AdcChannelConfig>* DaisySeedHAL::FindAnalogPin(uint8_t Pin) const
{
	for (auto& state : m_AnalogPins)
	{
		if (state.Pin != Pin)
			continue;

		return &state;
	}

	return nullptr;
}

DaisySeedHAL::PinState<daisy::AdcChannelConfig>* DaisySeedHAL::FindOrGetNewAnalogPin(uint8_t Pin)
{
	PinState<daisy::AdcChannelConfig>* state = FindAnalogPin(Pin);
	if (state != nullptr)
		return state;

	ASSERT(m_LastFreeAnalogPinIndex < (uint8_t)AnalogPins::COUNT, "Out of free Analog pins");

	return &m_AnalogPins[m_LastFreeAnalogPinIndex++];
}

DaisySeedHAL::PinState<daisy::GPIO>& DaisySeedHAL::GetDigitalPinState(uint8_t Pin)
{
	return m_DigitalPins[GetDigitalPinIndex(Pin)];
}

const DaisySeedHAL::PinState<daisy::GPIO>& DaisySeedHAL::GetDigitalPinState(uint8_t Pin) const
{
	return m_DigitalPins[GetDigitalPinIndex(Pin)];
}

uint8_t DaisySeedHAL::GetDigitalPinIndex(uint8_t Pin) const
{
	ASSERT(IsADigitalPin(Pin), "Pin %i is not an digital pin", Pin);

	return (uint8_t)Pin - (uint8_t)GPIOPins::Pin0;
}

DaisySeedHAL::PWMPinState* DaisySeedHAL::FindOrGetPWMPin(uint8_t Pin)
{
	for (auto& state : m_PWMPins)
	{
		if (state.State == nullptr || state.State->Pin != Pin)
			continue;

		return &state;
	}

	ASSERT(m_LastFreePWMPinIndex < (uint8_t)GPIOPins::COUNT, "Out of free PWM pin states");

	return &m_PWMPins[m_LastFreePWMPinIndex++];
}

#endif