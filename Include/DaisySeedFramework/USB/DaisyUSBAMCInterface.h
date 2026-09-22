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
#ifdef ENABLE_USB_AMC_DEBUG
private:
	struct DebugStats
	{
	public:
		uint32_t Underrun;
		uint32_t Overrun;
		uint32_t IsoIncomplete;
	};
#endif

public:
	DaisyUSBAMCInterface(DaisyUSBDevice* Device, const Configs& Configs, const AMCClassConfig& Class);
	
#ifdef ENABLE_USB_AMC_DEBUG
	void Update(void) override;
#endif

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

#ifdef ENABLE_USB_AMC_DEBUG
private:
	uint32_t m_LastLogTime;
	DebugStats m_DebugStatsIn;
	DebugStats m_DebugStatsOut;
#endif
};

template <typename T>
uint16_t DaisyUSBAMCInterface::PopSamples(T* InterleavedBuffer, uint16_t TotalSampleCount)
{
	ASSERT_ON_FLOATING_TYPE(T);
	ASSERT(InterleavedBuffer != nullptr, "InterleavedBuffer is null");
	ASSERT(TotalSampleCount != 0, "TotalSampleCount is zero");

	const uint8_t BytesPerSample = ((uint8_t)m_CurrentOutBitDepth / 8);
	const uint16_t AvailableSamples = (uint16_t)(m_ReceiveFIFO.GetSize() / BytesPerSample);
	const uint16_t SamplesToPop = Math::Min(AvailableSamples, TotalSampleCount);

	const float Gain = (m_CurrentIsOutMuted ? 0.0 : m_CurrentOutVolume);

#define POP_SAMPLE(BitsNum) \
	case BitDepths::BitDepths##BitsNum: \
	{ \
		for (uint16_t i = 0; i < SamplesToPop; ++i) \
		{ \
			int##BitsNum##_t pcm##BitsNum; \
			m_ReceiveFIFO.Pop(reinterpret_cast<uint8_t*>(&pcm##BitsNum), sizeof(int##BitsNum##_t)); \
			InterleavedBuffer[i] = ((T)pcm##BitsNum / INT##BitsNum##_MAX) * Gain; \
		} \
		break; \
	}

	switch (m_CurrentOutBitDepth)
	{
		POP_SAMPLE(8);
		POP_SAMPLE(16);
		POP_SAMPLE(24);
		POP_SAMPLE(32);
	}

	for (uint16_t i = SamplesToPop; i < TotalSampleCount; ++i)
		InterleavedBuffer[i] = 0;

#ifdef ENABLE_USB_AMC_DEBUG
	if (SamplesToPop < TotalSampleCount)
		m_DebugStatsOut.Underrun++;
#endif

	return SamplesToPop;

#undef POP_SAMPLE
}

template <typename T>
void DaisyUSBAMCInterface::PushSamples(const T* const InterleavedBuffer, uint16_t TotalSampleCount)
{
	ASSERT_ON_FLOATING_TYPE(T);

	ASSERT(InterleavedBuffer != nullptr, "InterleavedBuffer is null");
	ASSERT(TotalSampleCount != 0, "TotalSampleCount is zero");

	const uint8_t BytesPerSample = ((uint8_t)m_CurrentInBitDepth / 8);
	const uint16_t AvailableSamples = (uint16_t)(m_TransmitFIFO.GetFreeSpace() / BytesPerSample);
	const uint16_t SamplesToPush = Math::Min(AvailableSamples, TotalSampleCount);

	const float Gain = (m_CurrentIsInMuted ? 0.0 : m_CurrentInVolume);

#define PUSH_SAMPLE(BitsNum) \
	case BitDepths::BitDepths##BitsNum: \
	{ \
		for (uint16_t i = 0; i < SamplesToPush; ++i) \
		{ \
			T sample = Math::ClampSignal(InterleavedBuffer[i]); \
			int##BitsNum##_t pcm##BitsNum = (int##BitsNum##_t)(sample * INT##BitsNum##_MAX * Gain); \
			m_TransmitFIFO.Push(reinterpret_cast<const uint8_t*>(&pcm##BitsNum), sizeof(int##BitsNum##_t)); \
		} \
		break; \
	}

	switch (m_CurrentInBitDepth)
	{
		PUSH_SAMPLE(8);
		PUSH_SAMPLE(16);
		PUSH_SAMPLE(24);
		PUSH_SAMPLE(32);
	}

#ifdef ENABLE_USB_AMC_DEBUG
	if (SamplesToPush < TotalSampleCount)
		m_DebugStatsIn.Overrun++;
#endif

#undef PUSH_SAMPLE
}

#endif