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

	virtual void Read(float* InterleavedBuffer, uint16 TotalSampleCount)
	{
		PopSamples(InterleavedBuffer, TotalSampleCount);
	}
	virtual void Read(double* InterleavedBuffer, uint16 TotalSampleCount)
	{
		PopSamples(InterleavedBuffer, TotalSampleCount);
	}

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
	void OnDataOutStage(void) override;
	void OnDataInStage(void) override;
	void OnIsoOutIncomplete(void) override;
	void OnIsoInIncomplete(void) override;

	bool OnSetInterface(uint8 InterfaceIndex, uint8 AlternateSetting) override;
	uint8 GetCurrentAltSetting(uint8 InterfaceIndex) const override;
	void BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, uint16& BufferOffset, uint8 InterfaceIndex, const USBClassNode& Class) override;

	void TransmitBuffer(void);

	bool IsSampleRateSupported(uint32 Rate) const;

private:
	template <typename T>
	uint16 PopSamples(T* InterleavedBuffer, uint16 TotalSampleCount);
	template <typename T>
	void PushSamples(const T* const InterleavedBuffer, uint16 TotalSampleCount);

	void UpdatePacketSize(void);

public:
	static void BuildStreamingInterface(EP0Buffer& EP0Buffer, uint16& BufferOffset, uint8 InterfaceIndex, uint8 ChannelCount, uint8 Endpoint, uint8 TerminalLinkID, const AMCClassConfig& Config);

	static void CalculateStreamingInterfaceIndices(const Configs& Configs, const AMCClassConfig& Class, uint8& OutInterfaceIndex, uint8& InInterfaceIndex);
	static uint8 CalculateRequiredInterfaceCount(const AMCClassConfig& Class);

	static uint16 CalculateMaxPacketSize(uint8 ChannelCount, const AMCClassConfig& Class);

private:
	static uint16 CalculatePacketSize(uint8 ChannelCount, uint32 SampleRate, BitDepths BitDepth);

private:
	AMCClassConfig m_Class;

	uint8 m_OutAltSetting;
	uint8 m_InAltSetting;

	uint8 m_OutInterfaceIndex;
	uint8 m_InInterfaceIndex;

	uint8* m_ReceiveBuffer;
	StaticRingBuffer<uint8, sizeof(int32) * 1024, false> m_ReceiveFIFO;
	uint32 m_CurrentReceivePacketSize;

	uint8* m_TransmitBuffer;
	StaticRingBuffer<uint8, sizeof(int32) * 1024, false> m_TransmitFIFO;
	uint32 m_CurrentTransmitPacketSize;

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
uint16 DaisyUSBAMCInterface::PopSamples(T* InterleavedBuffer, uint16 TotalSampleCount)
{
	ASSERT_ON_FLOATING_TYPE(T);

	const uint8 bytesPerSample = ((uint8)m_CurrentOutBitDepth / 8);
	const size_t bytesToRead = TotalSampleCount * bytesPerSample;

	uint16 samplesPopped = 0;
	if (m_ReceiveFIFO.GetSize() >= bytesToRead)
		samplesPopped = TotalSampleCount;

	switch (m_CurrentOutBitDepth)
	{
	case BitDepths::BitDepths16:
	{
		if (m_ReceiveFIFO.GetSize() >= bytesToRead)
		{
			for (uint16 i = 0; i < TotalSampleCount; ++i)
			{
				int16_t pcm16;
				m_ReceiveFIFO.Pop(reinterpret_cast<uint8*>(&pcm16), sizeof(int16));

				InterleavedBuffer[i] = (T)(pcm16 / 32768.0);
			}
		}
		break;
	}

	case BitDepths::BitDepths24:
	{
		if (m_ReceiveFIFO.GetSize() >= bytesToRead)
		{
			for (uint16 i = 0; i < TotalSampleCount; ++i)
			{
				int24_t pcm24;
				m_ReceiveFIFO.Pop(reinterpret_cast<uint8*>(&pcm24), sizeof(int24_t));

				InterleavedBuffer[i] = (T)(pcm24 / 8388608.0);
			}
		}
		break;
	}

	case BitDepths::BitDepths32:
	{
		if (m_ReceiveFIFO.GetSize() >= bytesToRead)
		{
			for (uint16 i = 0; i < TotalSampleCount; ++i)
			{
				int32_t pcm32;
				m_ReceiveFIFO.Pop(reinterpret_cast<uint8*>(&pcm32), sizeof(int32_t));

				InterleavedBuffer[i] = (T)(pcm32 / 2147483648.0);
			}
		}
		break;
	}
	}

	for (uint16 i = samplesPopped; i < TotalSampleCount; ++i)
		InterleavedBuffer[i] = 0;

	return samplesPopped;
}

template <typename T>
void DaisyUSBAMCInterface::PushSamples(const T* const InterleavedBuffer, uint16 TotalSampleCount)
{
	ASSERT_ON_FLOATING_TYPE(T);

	ASSERT(InterleavedBuffer != nullptr, "InterleavedBuffer is null");
	ASSERT(TotalSampleCount != 0, "TotalSampleCount is zero");

	switch (m_CurrentInBitDepth)
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

			int24_t pcm24 = static_cast<int24_t>(sample * static_cast<T>(8388607.0));

			m_TransmitFIFO.Push(reinterpret_cast<const uint8*>(&pcm24), sizeof(int24_t));
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
	}
}

#endif