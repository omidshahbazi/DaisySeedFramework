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
	DaisyUSBAMCInterface(DaisyUSBDevice* Device, const Configs& Configs, const AMCClassConfig& Class);

	virtual void Read(float* InterleavedBuffer, uint16_t TotalSampleCount)
	{
		PopSamples(InterleavedBuffer, TotalSampleCount);
	}
	virtual void Read(double* InterleavedBuffer, uint16_t TotalSampleCount)
	{
		PopSamples(InterleavedBuffer, TotalSampleCount);
	}

	void Write(const float* const InterleavedBuffer, uint16_t TotalSampleCount) override
	{
		PushSamples(InterleavedBuffer, TotalSampleCount);
	}
	void Write(const double* const InterleavedBuffer, uint16_t TotalSampleCount) override
	{
		PushSamples(InterleavedBuffer, TotalSampleCount);
	}

	void SetControlChangedCallback(ControlChangedCallback Callback) override
	{
		m_ControlChangedCallback = Callback;
	}

	uint32_t GetCurrentOutSampleRate(void) const override
	{
		return m_CurrentOutSampleRate;
	}
	uint32_t GetCurrentInSampleRate(void) const override
	{
		return m_CurrentInSampleRate;
	}

	uint8_t GetCurrentOutBitDepth(void) const override
	{
		return (uint8_t)m_CurrentOutBitDepth;
	}
	uint8_t GetCurrentInBitDepth(void) const override
	{
		return (uint8_t)m_CurrentInBitDepth;
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
	void OnDeviceDataOutStage(void) override;
	void OnDataOutStage(void) override;
	void OnDataInStage(void) override;
	void OnIsoOutIncomplete(void) override;
	void OnIsoInIncomplete(void) override;

	bool OnSetInterface(uint8_t InterfaceIndex, uint8_t AlternateSetting) override;
	uint8_t GetCurrentAltSetting(uint8_t InterfaceIndex) const override;
	void BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, uint16_t& BufferOffset, uint8_t InterfaceIndex) const override;
	cstr GetDescriptorString(uint8_t StringIndex) const override;

	void TransmitBuffer(void);

	bool IsSampleRateSupported(uint32_t Rate) const;

private:
	template <typename T>
	uint16_t PopSamples(T* InterleavedBuffer, uint16_t TotalSampleCount);
	template <typename T>
	void PushSamples(const T* const InterleavedBuffer, uint16_t TotalSampleCount);

	void UpdatePacketSize(void);

public:
	static void BuildStreamingInterface(EP0Buffer& EP0Buffer, uint16_t& BufferOffset, uint8_t InterfaceIndex, uint8_t ChannelCount, uint8_t Endpoint, uint8_t TerminalLinkID, const AMCClassConfig& Config);

	static void CalculateStreamingInterfaceIndices(const Configs& Configs, const AMCClassConfig& Class, uint8_t& OutInterfaceIndex, uint8_t& InInterfaceIndex);
	static uint8_t CalculateRequiredInterfaceCount(const AMCClassConfig& Class);

	static uint16_t CalculateMaxPacketSize(uint8_t ChannelCount, const AMCClassConfig& Class);

private:
	static uint16_t CalculatePacketSize(uint8_t ChannelCount, uint32_t SampleRate, BitDepths BitDepth);

private:
	AMCClassConfig m_Class;

	uint8_t m_OutAltSetting;
	uint8_t m_InAltSetting;

	uint8_t m_OutInterfaceIndex;
	uint8_t m_InInterfaceIndex;

	uint8_t* m_ReceiveBuffer;
	StaticRingBuffer<uint8_t, sizeof(int32_t) * 1024, false> m_ReceiveFIFO;
	uint32_t m_CurrentReceivePacketSize;

	uint8_t* m_TransmitBuffer;
	StaticRingBuffer<uint8_t, sizeof(int32_t) * 1024, false> m_TransmitFIFO;
	uint32_t m_CurrentTransmitPacketSize;

	ControlChangedCallback m_ControlChangedCallback;

	uint32_t m_CurrentOutSampleRate;
	uint32_t m_CurrentInSampleRate;
	BitDepths m_CurrentOutBitDepth;
	BitDepths m_CurrentInBitDepth;
	LinearGain m_CurrentOutVolume;
	LinearGain m_CurrentInVolume;
	bool m_CurrentIsOutMuted;
	bool m_CurrentIsInMuted;
};

template <typename T>
uint16_t DaisyUSBAMCInterface::PopSamples(T* InterleavedBuffer, uint16_t TotalSampleCount)
{
	ASSERT_ON_FLOATING_TYPE(T);

	ASSERT(InterleavedBuffer != nullptr, "InterleavedBuffer is null");
	ASSERT(TotalSampleCount != 0, "TotalSampleCount is zero");

	const uint8_t bytesPerSample = ((uint8_t)m_CurrentOutBitDepth / 8);
	const size_t bytesToRead = TotalSampleCount * bytesPerSample;

	uint16_t samplesPopped = 0;
	if (m_ReceiveFIFO.GetSize() >= bytesToRead)
		samplesPopped = TotalSampleCount;

	const float Gain = (m_CurrentIsOutMuted ? 0.0 : m_CurrentOutVolume);

	switch (m_CurrentOutBitDepth)
	{
	case BitDepths::BitDepths8:
	{
		if (m_ReceiveFIFO.GetSize() >= bytesToRead)
		{
			for (uint16_t i = 0; i < TotalSampleCount; ++i)
			{
				int8_t pcm8;
				m_ReceiveFIFO.Pop(reinterpret_cast<uint8_t*>(&pcm8), sizeof(int8_t));

				InterleavedBuffer[i] = ((T)pcm8 / INT8_MAX) * Gain;
			}
		}
		break;
	}

	case BitDepths::BitDepths16:
	{
		if (m_ReceiveFIFO.GetSize() >= bytesToRead)
		{
			for (uint16_t i = 0; i < TotalSampleCount; ++i)
			{
				int16_t pcm16;
				m_ReceiveFIFO.Pop(reinterpret_cast<uint8_t*>(&pcm16), sizeof(int16_t));

				InterleavedBuffer[i] = ((T)pcm16 / INT16_MAX) * Gain;
			}
		}
		break;
	}

	case BitDepths::BitDepths24:
	{
		if (m_ReceiveFIFO.GetSize() >= bytesToRead)
		{
			for (uint16_t i = 0; i < TotalSampleCount; ++i)
			{
				int24_t pcm24;
				m_ReceiveFIFO.Pop(reinterpret_cast<uint8_t*>(&pcm24), sizeof(int24_t));

				InterleavedBuffer[i] = ((T)pcm24 / INT24_MAX) * Gain;;
			}
		}
		break;
	}

	case BitDepths::BitDepths32:
	{
		if (m_ReceiveFIFO.GetSize() >= bytesToRead)
		{
			for (uint16_t i = 0; i < TotalSampleCount; ++i)
			{
				int32_t pcm32;
				m_ReceiveFIFO.Pop(reinterpret_cast<uint8_t*>(&pcm32), sizeof(int32_t));

				InterleavedBuffer[i] = ((T)pcm32 / INT32_MAX) * Gain;;
			}
		}
		break;
	}
	}

	for (uint16_t i = samplesPopped; i < TotalSampleCount; ++i)
		InterleavedBuffer[i] = 0;

	return samplesPopped;
}

template <typename T>
void DaisyUSBAMCInterface::PushSamples(const T* const InterleavedBuffer, uint16_t TotalSampleCount)
{
	ASSERT_ON_FLOATING_TYPE(T);

	ASSERT(InterleavedBuffer != nullptr, "InterleavedBuffer is null");
	ASSERT(TotalSampleCount != 0, "TotalSampleCount is zero");

	const float Gain = (m_CurrentIsInMuted ? 0.0 : m_CurrentInVolume);

	switch (m_CurrentInBitDepth)
	{
	case BitDepths::BitDepths8:
	{
		for (uint16_t i = 0; i < TotalSampleCount; ++i)
		{
			T sample = Math::ClampSignal(InterleavedBuffer[i]);

			int8_t pcm8 = (int8_t)(sample * INT8_MAX * Gain);

			m_TransmitFIFO.Push(reinterpret_cast<const uint8_t*>(&pcm8), sizeof(int8_t));
		}
		break;
	}

	case BitDepths::BitDepths16:
	{
		for (uint16_t i = 0; i < TotalSampleCount; ++i)
		{
			T sample = Math::ClampSignal(InterleavedBuffer[i]);

			int16_t pcm16 = (int16_t)(sample * INT16_MAX * Gain);

			m_TransmitFIFO.Push(reinterpret_cast<const uint8_t*>(&pcm16), sizeof(int16_t));
		}
		break;
	}

	case BitDepths::BitDepths24:
	{
		for (uint16_t i = 0; i < TotalSampleCount; ++i)
		{
			T sample = Math::ClampSignal(InterleavedBuffer[i]);

			int24_t pcm24 = (int24_t)(sample * INT24_MAX * Gain);

			m_TransmitFIFO.Push(reinterpret_cast<const uint8_t*>(&pcm24), sizeof(int24_t));
		}
		break;
	}

	case BitDepths::BitDepths32:
	{
		for (uint16_t i = 0; i < TotalSampleCount; ++i)
		{
			T sample = Math::ClampSignal(InterleavedBuffer[i]);

			int32_t pcm32 = (int32_t)(sample * INT32_MAX * Gain);

			m_TransmitFIFO.Push(reinterpret_cast<const uint8_t*>(&pcm32), sizeof(int32_t));
		}
		break;
	}
	}
}

#endif