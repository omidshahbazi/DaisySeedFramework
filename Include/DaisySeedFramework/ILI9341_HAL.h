#pragma once
#ifndef ILI9341_HAL_H
#define ILI9341_HAL_H

#include "Common.h"
#include "I_LCD_HAL.h"
#include "DaisyInclude.h"
#include <DigitalSignalProcessing/IHAL.h>
#include <DigitalSignalProcessing/FixedFunction.h>

class ILI9341_HAL : public I_LCD_HAL
{
private:
	static constexpr uint8_t MAX_FRAME_RATE = 60;

	static constexpr uint16_t FRAME_BUFFER_CHUNK_COUNT = 10;
	static_assert(FRAME_BUFFER_CHUNK_COUNT > 2, "FRAME_BUFFER_CHUNK_COUNT must be greater than 2, cause the HAL_SPI_Transmit_DMA accepts the length as uint16_t");

public:
	typedef FixedFunction<void(void)> RenderEventHandler;

public:
	ILI9341_HAL(IHAL* HAL, GPIOPins SCLK, GPIOPins MOSI, GPIOPins NSS, GPIOPins DC, GPIOPins RST, Orientations Orientation, Point PixelSize);

	void Initialize(void);

	void SetOnRender(RenderEventHandler Listener)
	{
		m_RenderListener = Listener;
	}

	void Update(void) override;

	void SetTargetFrameRate(uint8_t Value);

	uint8_t GetTargetFrameRate(void) const
	{
		return m_TargetFrameRate;
	}

	void Clear(Color Color) override;

	void DrawPixel(Point Position, Color Color) override;

	const Point& GetDimension(void) const override
	{
		return m_Dimension;
	}

private:
	void PaintPixel(int16_t X, int16_t Y, uint16_t R5G6B5, uint8_t Alpha);

	void InitializeSPI(GPIOPins SCLK, GPIOPins MOSI, GPIOPins NSS, GPIOPins DC, GPIOPins RST);

	void InitDriver(Orientations Orientation);

	uint8_t SetOrientationAndGetTheRotationBits(Orientations Orientation);

	void SendCommand(uint8_t Command);

	void SendData(uint8_t* Buffer, uint32_t Size);

	void SetAddressWindow(uint16_t X0, uint16_t Y0, uint16_t X1, uint16_t Y1);

	void UpdateDataDMA(void);

	static void OnDMATransmissionCompleted(void* Context, daisy::SpiHandle::Result Result);

private:
	IHAL* m_HAL;
	GPIOPins m_PinSCLK, m_PinMOSI, m_PinNSS, m_PinDC, m_PinRST;
	Point m_PixelSize;
	Orientations m_Orientation;

	RenderEventHandler m_RenderListener;

	uint32_t m_FrameBufferChunkSize;
	uint16_t* m_FrameBuffer;
	bool* m_FrameBufferDirty;

	daisy::SpiHandle m_SPI;

	daisy::GPIO m_RST;
	daisy::GPIO m_DC;
	daisy::GPIO m_CS;

	uint8_t m_TargetFrameRate;
	Point m_Dimension;

	uint16_t m_UpdateStep;
	uint32_t m_NextUpdateTime;
	bool m_IsDMABusy;
	uint8_t m_LastFrameBufferDirtyIndex;
};

#endif