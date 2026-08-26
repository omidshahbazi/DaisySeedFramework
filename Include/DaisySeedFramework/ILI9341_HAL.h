#pragma once
#ifndef ILI9341_HAL_H
#define ILI9341_HAL_H

#include "Common.h"
#include "I_LCD_HAL.h"
#include "DaisyInclude.h"
#include <DigitalSignalProcessing/IHAL.h>
#include <DigitalSignalProcessing/ContextCallback.h>

class ILI9341_HAL : public I_LCD_HAL
{
private:
	static constexpr uint8 MAX_FRAME_RATE = 60;

	static constexpr uint16 FRAME_BUFFER_CHUNK_COUNT = 10;
	static_assert(FRAME_BUFFER_CHUNK_COUNT > 2, "FRAME_BUFFER_CHUNK_COUNT must be greater than 2, cause the HAL_SPI_Transmit_DMA accepts the length as uint16");

public:
	typedef ContextCallback<void> RenderEventHandler;

public:
	ILI9341_HAL(IHAL* HAL, GPIOPins SCLK, GPIOPins MOSI, GPIOPins NSS, GPIOPins DC, GPIOPins RST, Orientations Orientation, Point PixelSize);

	void Initialize(void);

	void SetOnRender(RenderEventHandler Listener)
	{
		m_RenderListener = Listener;
	}

	void Update(void) override;

	void SetTargetFrameRate(uint8 Value);

	uint8 GetTargetFrameRate(void) const
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
	void PaintPixel(int16 X, int16 Y, uint16 R5G6B5, uint8 Alpha);

	void InitializeSPI(GPIOPins SCLK, GPIOPins MOSI, GPIOPins NSS, GPIOPins DC, GPIOPins RST);

	void InitDriver(Orientations Orientation);

	uint8 SetOrientationAndGetTheRotationBits(Orientations Orientation);

	void SendCommand(uint8 Command);

	void SendData(uint8* Buffer, uint32 Size);

	void SetAddressWindow(uint16 X0, uint16 Y0, uint16 X1, uint16 Y1);

	void UpdateDataDMA(void);

	static void OnDMATransmissionCompleted(void* Context, daisy::SpiHandle::Result Result);

private:
	IHAL* m_HAL;
	GPIOPins m_PinSCLK, m_PinMOSI, m_PinNSS, m_PinDC, m_PinRST;
	Point m_PixelSize;
	Orientations m_Orientation;

	RenderEventHandler m_RenderListener;

	uint32 m_FrameBufferChunkSize;
	uint16* m_FrameBuffer;
	bool* m_FrameBufferDirty;

	daisy::SpiHandle m_SPI;

	daisy::GPIO m_RST;
	daisy::GPIO m_DC;
	daisy::GPIO m_CS;

	uint8 m_TargetFrameRate;
	Point m_Dimension;

	uint16 m_UpdateStep;
	uint32 m_NextUpdateTime;
	bool m_IsDMABusy;
	uint8 m_LastFrameBufferDirtyIndex;
};

#endif