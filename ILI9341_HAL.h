#pragma once
#ifndef ILI9341_HAL_H
#define ILI9341_HAL_H
#pragma GCC push_options
#pragma GCC optimize("Os")

#include "I_LCD_HAL.h"
#include "DaisySeedHAL.h"
#include "DSP/Math.h"
#include "DSP/ContextCallback.h"
#include <daisy_seed.h>

template <uint32 Width, uint32 Height>
class ILI9341_HAL : public I_LCD_HAL
{
	static_assert(Width != 0, "Width must be greater than zero");
	static_assert(Height != 0, "Height must be greater than zero");

	static constexpr uint8 MAX_FRAME_RATE = 60;
	static constexpr uint32 FRAME_BUFFER_LENGTH = Width * Height;
	static constexpr uint16 FRAME_BUFFER_CHUNK_COUNT = 10;
	static constexpr uint32 FRAME_BUFFER_CHUNK_SIZE = FRAME_BUFFER_LENGTH / FRAME_BUFFER_CHUNK_COUNT;

	static_assert(FRAME_BUFFER_CHUNK_COUNT > 2, "FRAME_BUFFER_CHUNK_COUNT must be greater than 2, cause the HAL_SPI_Transmit_DMA accepts the length as uint16");

public:
	typedef ContextCallback<void> RenderEventHandler;

public:
	ILI9341_HAL(IHAL *HAL, GPIOPins SCLK, GPIOPins MOSI, GPIOPins NSS, GPIOPins DC, GPIOPins RST, Orientations Orientation)
		: m_HAL(HAL),
		  m_PinSCLK(SCLK),
		  m_PinMOSI(MOSI),
		  m_PinNSS(NSS),
		  m_PinDC(DC),
		  m_PinRST(RST),
		  m_Orientation(Orientation),
		  m_FrameBuffer(nullptr),
		  m_FrameBufferDirty(nullptr),
		  m_TargetFrameRate(0),
		  m_NextUpdateTime(0),
		  m_IsDMABusy(false),
		  m_LastFrameBufferDirtyIndex(0)
	{
		ASSERT(m_HAL != nullptr, "m_HAL cannot be null");
	}

	void Initialize(void)
	{
		m_FrameBuffer = Memory::Allocate<uint16>(FRAME_BUFFER_LENGTH, true);
		m_FrameBufferDirty = Memory::Allocate<bool>(FRAME_BUFFER_CHUNK_COUNT, true);

		InitializeSPI(m_PinSCLK, m_PinMOSI, m_PinNSS, m_PinDC, m_PinRST);

		InitDriver(m_Orientation);

		SetTargetFrameRate(MAX_FRAME_RATE);
	}

	void SetOnRender(RenderEventHandler Listener)
	{
		m_RenderListener = Listener;
	}

	void Update(void) override
	{
		if (m_IsDMABusy)
			return;

		float time = m_HAL->GetTimeSinceStartup();
		if (time < m_NextUpdateTime)
			return;
		m_NextUpdateTime = time + m_TargetFrameRate;

		m_RenderListener();

		UpdateDataDMA();
	}

	void SetTargetFrameRate(uint8 Value)
	{
		ASSERT(Value != 0, "Invalid Value");

		m_TargetFrameRate = 1.0 / Math::Min(MAX_FRAME_RATE, Value);
	}

	void Clear(Color Color) override
	{
		uint16 color = Color.R5G6B5();

		for (uint32 y = 0; y < m_Dimension.Y; ++y)
			for (uint32 x = 0; x < m_Dimension.X; ++x)
				m_FrameBuffer[x + (y * m_Dimension.X)] = SWAP_ENDIAN_16BIT(color);

		Memory::Set(m_FrameBufferDirty, 1, FRAME_BUFFER_CHUNK_COUNT);
	}

	void DrawPixel(Point Position, Color Color) override
	{
		if (Position.X < 0 || Position.Y < 0 ||
			Position.X >= m_Dimension.X || Position.Y >= m_Dimension.Y)
			return;

		PaintPixel(Position.X, Position.Y, Color.R5G6B5(), Color.A);
	}

	const Point &GetDimension(void) const override
	{
		return m_Dimension;
	}

private:
	void PaintPixel(uint16 X, uint16 Y, uint16 R5G6B5, uint8 Alpha)
	{
		uint32 index = X + (Y * m_Dimension.X);

		if (Alpha != 255)
		{
			uint16 currentColor = SWAP_ENDIAN_16BIT(m_FrameBuffer[index]);
			R5G6B5 = Color::BlendR5G6B5(R5G6B5, currentColor, Alpha);
		}

		m_FrameBuffer[index] = SWAP_ENDIAN_16BIT(R5G6B5);

		m_FrameBufferDirty[index / FRAME_BUFFER_CHUNK_SIZE] = true;
	}

	void InitializeSPI(GPIOPins SCLK, GPIOPins MOSI, GPIOPins NSS, GPIOPins DC, GPIOPins RST)
	{
		daisy::SpiHandle::Config spiConfig;
		spiConfig.periph = daisy::SpiHandle::Config::Peripheral::SPI_1;
		spiConfig.mode = daisy::SpiHandle::Config::Mode::MASTER;
		spiConfig.direction = daisy::SpiHandle::Config::Direction::TWO_LINES_TX_ONLY;
		spiConfig.clock_polarity = daisy::SpiHandle::Config::ClockPolarity::LOW;
		spiConfig.baud_prescaler = daisy::SpiHandle::Config::BaudPrescaler::PS_2;
		spiConfig.clock_phase = daisy::SpiHandle::Config::ClockPhase::ONE_EDGE;
		spiConfig.nss = daisy::SpiHandle::Config::NSS::HARD_OUTPUT;
		spiConfig.datasize = 8;
		spiConfig.pin_config.sclk = DaisySeedHALBase::GetPin((uint8)SCLK);
		spiConfig.pin_config.mosi = DaisySeedHALBase::GetPin((uint8)MOSI);
		spiConfig.pin_config.nss = DaisySeedHALBase::GetPin((uint8)NSS);

		m_SPI.Init(spiConfig);

		daisy::GPIO::Config pinConfig;
		pinConfig.mode = daisy::GPIO::Mode::OUTPUT;

		pinConfig.pin = DaisySeedHALBase::GetPin((uint8)DC);
		m_DC.Init(pinConfig);

		pinConfig.pin = DaisySeedHALBase::GetPin((uint8)RST);
		m_RST.Init(pinConfig);

		pinConfig.pin = DaisySeedHALBase::GetPin((uint8)NSS);
		m_CS.Init(pinConfig);
		m_CS.Write(0);
	}

	void InitDriver(Orientations Orientation)
	{
		m_RST.Write(0);
		m_HAL->Delay(100);

		m_RST.Write(1);
		m_HAL->Delay(100);

		// Software Reset
		SendCommand(0x01);
		m_HAL->Delay(100);

		// command list is based on https://github.com/martnak/STM32-ILI9341

		// POWER CONTROL A
		SendCommand(0xCB);
		{
			uint8 data[5] = {0x39, 0x2C, 0x00, 0x34, 0x02};
			SendData(data, 5);
		}

		// POWER CONTROL B
		SendCommand(0xCF);
		{
			uint8 data[3] = {0x00, 0xC1, 0x30};
			SendData(data, 3);
		}

		// DRIVER TIMING CONTROL A
		SendCommand(0xE8);
		{
			uint8 data[3] = {0x85, 0x00, 0x78};
			SendData(data, 3);
		}

		// DRIVER TIMING CONTROL B
		SendCommand(0xEA);
		{
			uint8 data[2] = {0x00, 0x00};
			SendData(data, 2);
		}

		// POWER ON SEQUENCE CONTROL
		SendCommand(0xED);
		{
			uint8 data[4] = {0x64, 0x03, 0x12, 0x81};
			SendData(data, 4);
		}

		// PUMP RATIO CONTROL
		SendCommand(0xF7);
		{
			uint8 data[1] = {0x20};
			SendData(data, 1);
		}

		// POWER CONTROL,VRH[5:0]
		SendCommand(0xC0);
		{
			uint8 data[1] = {0x23};
			SendData(data, 1);
		}

		// POWER CONTROL,SAP[2:0];BT[3:0]
		SendCommand(0xC1);
		{
			uint8 data[1] = {0x10};
			SendData(data, 1);
		}

		// VCM CONTROL
		SendCommand(0xC5);
		{
			uint8 data[2] = {0x3E, 0x28};
			SendData(data, 2);
		}

		// VCM CONTROL 2
		SendCommand(0xC7);
		{
			uint8 data[1] = {0x86};
			SendData(data, 1);
		}

		// MEMORY ACCESS CONTROL
		SendCommand(0x36);
		{
			uint8 data[1] = {0x48};
			SendData(data, 1);
		}

		// PIXEL FORMAT
		SendCommand(0x3A);
		{
			uint8 data[1] = {0x55};
			SendData(data, 1);
		}

		// FRAME RATIO CONTROL, STANDARD RGB COLOR
		SendCommand(0xB1);
		{
			uint8 data[2] = {0x00, 0x18};
			SendData(data, 2);
		}

		// DISPLAY FUNCTION CONTROL
		SendCommand(0xB6);
		{
			uint8 data[3] = {0x08, 0x82, 0x27};
			SendData(data, 3);
		}

		// 3GAMMA FUNCTION DISABLE
		SendCommand(0xF2);
		{
			uint8 data[1] = {0x00};
			SendData(data, 1);
		}

		// GAMMA CURVE SELECTED
		SendCommand(0x26);
		{
			uint8 data[1] = {0x01};
			SendData(data, 1);
		}

		// POSITIVE GAMMA CORRECTION
		SendCommand(0xE0);
		{
			uint8 data[15] = {0x0F,
							  0x31,
							  0x2B,
							  0x0C,
							  0x0E,
							  0x08,
							  0x4E,
							  0xF1,
							  0x37,
							  0x07,
							  0x10,
							  0x03,
							  0x0E,
							  0x09,
							  0x00};
			SendData(data, 15);
		}

		// NEGATIVE GAMMA CORRECTION
		SendCommand(0xE1);
		{
			uint8 data[15] = {0x00,
							  0x0E,
							  0x14,
							  0x03,
							  0x11,
							  0x07,
							  0x31,
							  0xC1,
							  0x48,
							  0x08,
							  0x0F,
							  0x0C,
							  0x31,
							  0x36,
							  0x0F};
			SendData(data, 15);
		}

		// EXIT SLEEP
		SendCommand(0x11);
		m_HAL->Delay(100);

		// TURN ON DISPLAY
		SendCommand(0x29);

		// MADCTL
		SendCommand(0x36);
		m_HAL->Delay(10);
		{
			uint8 data[1] = {SetOrientationAndGetTheRotationBits(Orientation)};
			SendData(data, 1);
		}
	}

	uint8 SetOrientationAndGetTheRotationBits(Orientations Orientation)
	{
		uint8 ili_bgr = 0x08;
		uint8 ili_mx = 0x40;
		uint8 ili_my = 0x80;
		uint8 ili_mv = 0x20;

		uint8 rotationBits = ili_bgr;

		switch (Orientation)
		{
		case Orientations::Upright:
		{
			m_Dimension = {Width, Height};
			rotationBits |= ili_mx | ili_my | ili_mv;
		}
		break;

		case Orientations::UpsideDown:
		{
			m_Dimension = {Height, Width};
			rotationBits |= ili_my;
		}
		break;

		case Orientations::ToRight:
		{
			m_Dimension = {Width, Height};
			rotationBits |= ili_mx | ili_my | ili_mv;
		}
		break;

		case Orientations::ToLeft:
		{
			m_Dimension = {Width, Height};
			rotationBits |= ili_mv;
		}
		break;
		}

		return rotationBits;
	}

	void SendCommand(uint8 Command)
	{
		m_DC.Write(0);

		m_SPI.BlockingTransmit(&Command, 1);
	}

	void SendData(uint8 *Buffer, uint32 Size)
	{
		m_DC.Write(1);

		m_SPI.BlockingTransmit(Buffer, Size);
	}

	void SetAddressWindow(uint16 X0, uint16 Y0, uint16 X1, uint16 Y1)
	{
		// Column address set
		SendCommand(0x2A); // CASET
		{
			uint8 data[4] = {static_cast<uint8>((X0 >> 8) & 0xFF),
							 static_cast<uint8>(X0 & 0xFF),
							 static_cast<uint8>((X1 >> 8) & 0xFF),
							 static_cast<uint8>(X1 & 0xFF)};
			SendData(data, 4);
		}

		// Row address set
		SendCommand(0x2B); // RASET
		{
			uint8 data[4] = {static_cast<uint8>((Y0 >> 8) & 0xFF),
							 static_cast<uint8>(Y0 & 0xFF),
							 static_cast<uint8>((Y1 >> 8) & 0xFF),
							 static_cast<uint8>(Y1 & 0xFF)};
			SendData(data, 4);
		}

		// Write to RAM
		SendCommand(0x2C); // RAMWR
	}

	void UpdateDataDMA(void)
	{
		bool found = false;
		for (uint8 i = m_LastFrameBufferDirtyIndex; i < FRAME_BUFFER_CHUNK_COUNT; ++i)
		{
			if (!m_FrameBufferDirty[i])
				continue;

			m_LastFrameBufferDirtyIndex = i;
			found = true;
			break;
		}

		if (!found)
			return;

		const uint16 chunkHeight = FRAME_BUFFER_CHUNK_SIZE / m_Dimension.X;

		const uint16 x0 = 0;
		const uint16 y0 = m_LastFrameBufferDirtyIndex * chunkHeight;
		const uint16 x1 = m_Dimension.X - 1;
		const uint16 y1 = y0 + chunkHeight;
		SetAddressWindow(x0, y0, x1, y1);

		m_IsDMABusy = true;

		uint8 *data = reinterpret_cast<uint8 *>(m_FrameBuffer + (m_LastFrameBufferDirtyIndex * FRAME_BUFFER_CHUNK_SIZE));
		uint32 length = FRAME_BUFFER_CHUNK_SIZE * sizeof(uint16);

		dsy_dma_clear_cache_for_buffer(data, length);

		m_DC.Write(1);

		m_SPI.DmaTransmit(data, length, nullptr, &OnDMATransmissionCompleted, this);
	}

	static void OnDMATransmissionCompleted(void *Context, daisy::SpiHandle::Result Result)
	{
		if (Result != daisy::SpiHandle::Result::OK)
			return;

		ILI9341_HAL *thisPtr = static_cast<ILI9341_HAL *>(Context);

		thisPtr->m_FrameBufferDirty[thisPtr->m_LastFrameBufferDirtyIndex] = false;

		if (++thisPtr->m_LastFrameBufferDirtyIndex == FRAME_BUFFER_CHUNK_COUNT)
		{
			thisPtr->m_IsDMABusy = false;
			thisPtr->m_LastFrameBufferDirtyIndex = 0;

			return;
		}

		thisPtr->UpdateDataDMA();
	}

private:
	IHAL *m_HAL;
	GPIOPins m_PinSCLK, m_PinMOSI, m_PinNSS, m_PinDC, m_PinRST;
	Orientations m_Orientation;

	RenderEventHandler m_RenderListener;

	uint16 *m_FrameBuffer;
	bool *m_FrameBufferDirty;

	daisy::SpiHandle m_SPI;

	daisy::GPIO m_RST;
	daisy::GPIO m_DC;
	daisy::GPIO m_CS;

	float m_TargetFrameRate;
	Point m_Dimension;

	float m_NextUpdateTime;
	bool m_IsDMABusy;
	uint8 m_LastFrameBufferDirtyIndex;
};

typedef ILI9341_HAL<320, 240> ILI9341_HAL_320_240;

#endif
#pragma GCC pop_options