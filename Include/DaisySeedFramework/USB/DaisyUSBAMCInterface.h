#pragma once
#ifndef DAISY_USB_AMC_INTERFACE_H
#define DAISY_USB_AMC_INTERFACE_H

#include "DaisySeedFramework/USB/DaisyUSBInterfaceCommon.h"
#include "DaisySeedFramework/USB/USBAMCDefinitions.h"
#include "DaisySeedFramework/StaticRingBuffer.h"
#include <DigitalSignalProcessing/USB/IUSBAMCInterface.h>
#include <DigitalSignalProcessing/Math.h>

class DaisyUSBAMCInterface : public IUSBAMCInterface, public DaisyUSBInterfaceCommon
{
public:
	DaisyUSBAMCInterface(DaisyUSB* USB, const Configs& Configs, const AMCClassConfig& Class);

	void Write(const float* const InterleavedBuffer, uint16 TotalSampleCount) override
	{
		PushSamples(InterleavedBuffer, TotalSampleCount);
	}
	void Write(const double* const InterleavedBuffer, uint16 TotalSampleCount) override
	{
		PushSamples(InterleavedBuffer, TotalSampleCount);
	}

	void SetControlChangedCallback(ControlChangedCallback Callback) override
	{
		m_ControlChangedCallback = Callback;
	}

	uint32 GetCurrentOutSampleRate(void) const override
	{
		return m_CurrentOutSampleRate;
	}
	uint32 GetCurrentInSampleRate(void) const override
	{
		return m_CurrentInSampleRate;
	}

	uint8 GetCurrentOutBitDepth(void) const override
	{
		return (uint8)m_CurrentOutBitDepth;
	}
	uint8 GetCurrentInBitDepth(void) const override
	{
		return (uint8)m_CurrentInBitDepth;
	}

	dBGain GetCurrentOutVolume(void) const override
	{
		return m_CurrentOutVolume;
	}
	dBGain GetCurrentInVolume(void) const override
	{
		return m_CurrentInVolume;
	}

	bool GetIsOutMuted(void) const override
	{
		return m_CurrentIsOutMuted;
	}
	bool GetIsInMuted(void) const override
	{
		return m_CurrentIsInMuted;
	}

	bool OnSetupStage(const USBDeviceSetupPacket* Setup) override;
	void OnSetupCompleted(void) override;
	void OnDataInStage(void) override;
	void OnDataOutStage(void) override;
	bool OnSetInterface(uint8 InterfaceIndex, uint8 AlternateSetting) override;
	uint8 GetCurrentAltSetting(uint8 InterfaceIndex) const override;
	void BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, uint16& BufferOffset, uint8 InterfaceIndex, const USBClassNode& Class) override;

	void TransmitBuffer(void);

	bool IsSampleRateSupported(uint32 Rate) const;

private:
	template <typename T>
	void PushSamples(const T* const InterleavedBuffer, uint16 TotalSampleCount);

public:
	static void BuildStreamingInterface(EP0Buffer& EP0Buffer, uint16& BufferOffset, uint8 InterfaceIndex, uint8 ChannelCount, uint8 Endpoint, uint8 TerminalLinkID, const AMCClassConfig& Config, bool IsOutput);

	static void CalculateStreamingInterfaceIndices(const Configs& Configs, const AMCClassConfig& Class, uint8& OutInterfaceIndex, uint8& InInterfaceIndex);
	static uint8 CalculateRequiredInterfaceCount(const AMCClassConfig& Class);

	static uint16 CalculateIsoPacketSize(uint8 ChannelCount, const AMCClassConfig& Class);

private:
	AMCClassConfig m_Class;

	uint8 m_OutAltSetting;
	uint8 m_InAltSetting;

	uint8 m_OutInterfaceIndex;
	uint8 m_InInterfaceIndex;

	uint8* m_TransmitBuffer;
	StaticRingBuffer<uint8, 4096> m_TransmitFIFO;

	ControlChangedCallback m_ControlChangedCallback;

	uint32 m_CurrentOutSampleRate;
	uint32 m_CurrentInSampleRate;
	BitDepths m_CurrentOutBitDepth;
	BitDepths m_CurrentInBitDepth;
	dBGain m_CurrentOutVolume;
	dBGain m_CurrentInVolume;
	bool m_CurrentIsOutMuted;
	bool m_CurrentIsInMuted;
};

template <typename T>
void DaisyUSBAMCInterface::PushSamples(const T* const InterleavedBuffer, uint16 TotalSampleCount)
{
	ASSERT(InterleavedBuffer != nullptr, "InterleavedBuffer is null");
	ASSERT(TotalSampleCount != 0, "TotalSampleCount is zero");

	const BitDepths bitDepth = m_CurrentInBitDepth;

	switch (bitDepth)
	{
	case BitDepths::BitDepths16:
	{
		for (uint16 i = 0; i < TotalSampleCount; ++i)
		{
			T sample = Math::ClampSignal(InterleavedBuffer[i]);

			int16 pcm16 = static_cast<int16>(sample * static_cast<T>(32767.0));

			m_TransmitFIFO.Push(reinterpret_cast<const uint8*>(&pcm16), sizeof(int16));
		}
		break;
	}

	case BitDepths::BitDepths24:
	{
		for (uint16 i = 0; i < TotalSampleCount; ++i)
		{
			T sample = Math::ClampSignal(InterleavedBuffer[i]);

			uint24 pcm24 = static_cast<int32>(sample * static_cast<T>(8388607.0));

			m_TransmitFIFO.Push(pcm24.bytes, sizeof(pcm24.bytes));
		}
		break;
	}

	case BitDepths::BitDepths32:
	{
		for (uint16 i = 0; i < TotalSampleCount; ++i)
		{
			T sample = Math::ClampSignal(InterleavedBuffer[i]);

			int32 pcm32 = static_cast<int32>(sample * static_cast<T>(2147483647.0));

			m_TransmitFIFO.Push(reinterpret_cast<const uint8*>(&pcm32), sizeof(int32));
		}
		break;
	}

	default:
		break;
	}
}

#endif