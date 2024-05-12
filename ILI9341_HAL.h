#pragma once

#include "I_LCD_HAL.h"
#include "DaisySeedHAL.h"
#include "DSP/Math.h"
#include <daisy_seed.h>

template <uint32 Width, uint32 Height>
class ILI9341_HAL : public I_LCD_HAL
{
public:
	ILI9341_HAL(DaisySeedHAL *DaisyHAL, GPIOPins SCLK, GPIOPins MOSI, GPIOPins NSS, GPIOPins DC, GPIOPins RST, Orientations Orientation)
		: m_DaisyHAL(DaisyHAL),
		  m_PinSCLK(SCLK),
		  m_PinMOSI(MOSI),
		  m_PinNSS(NSS),
		  m_PinDC(DC),
		  m_PinRST(RST),
		  m_Orientation(Orientation),
		  m_ColorBuffer(nullptr),
		  m_FrameBuffer(nullptr),
		  m_TargetFrameRate(0),
		  m_RotatedWidth(0),
		  m_RotatedHeight(0),
		  m_NextUpdateTime(0),
		  m_IsDMABusy(false),
		  m_RemainingBufferLengthToSend(0)
	{
	}

	void Init(void)
	{
		m_ColorBuffer = reinterpret_cast<uint8 *>(m_DaisyHAL->Allocate(COLOR_BUFFER_LENGTH, true));

		// TODO: Shouldn't this be on the DMA_BUFFER instead of SDRAM_BUFFER? IT used to be originally, but it also work this way, investigate more
		m_FrameBuffer = reinterpret_cast<uint8 *>(m_DaisyHAL->Allocate(FRAME_BUFFER_LENGTH, true));

		InitializeSPI(m_PinSCLK, m_PinMOSI, m_PinNSS, m_PinDC, m_PinRST);

		InitDriver(m_Orientation);

		SetTargetFrameRate(60);
	}

	void Update(void) override
	{
		if (m_IsDMABusy)
			return;

		if (m_DaisyHAL->GetTimeSinceStartup() < m_NextUpdateTime)
			return;
		m_NextUpdateTime = m_DaisyHAL->GetTimeSinceStartup() + m_TargetFrameRate;

		SetAddressWindow(0, 0, m_RotatedWidth - 1, m_RotatedHeight - 1);

		SendTheEntireDataDMA();
	}

	void SetTargetFrameRate(uint8 Value)
	{
		m_TargetFrameRate = 1.0 / Value;
	}

	void Clear(Color Color) override
	{
		// TODO: This can be desolved into two different memsets to fill the buffer arrays

		uint16 color = Color.R5G6B5();

		for (uint32 y = 0; y < m_RotatedHeight; ++y)
			for (uint32 x = 0; x < m_RotatedWidth; ++x)
				PaintPixel(x, y, color, Color.A);
	}

	void DrawPixel(uint16 X, uint16 Y, Color Color) override
	{
		if (X < 0 || Y < 0 ||
			X >= m_RotatedWidth || Y >= m_RotatedHeight)
			return;

		PaintPixel(X, Y, Color.R5G6B5(), Color.A);
	}

private:
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
		spiConfig.pin_config.sclk = DaisySeedHAL::GetPin((uint8)SCLK);
		spiConfig.pin_config.mosi = DaisySeedHAL::GetPin((uint8)MOSI);
		spiConfig.pin_config.nss = DaisySeedHAL::GetPin((uint8)NSS);

		m_SPI.Init(spiConfig);

		daisy::GPIO::Config pinConfig;
		pinConfig.mode = daisy::GPIO::Mode::OUTPUT;

		pinConfig.pin = DaisySeedHAL::GetPin((uint8)DC);
		m_DC.Init(pinConfig);

		pinConfig.pin = DaisySeedHAL::GetPin((uint8)RST);
		m_RST.Init(pinConfig);

		pinConfig.pin = DaisySeedHAL::GetPin((uint8)NSS);
		m_CS.Init(pinConfig);
		m_CS.Write(0);
	}

	void InitDriver(Orientations Orientation)
	{
		Reset();

		// Software Reset
		SendCommand(0x01);
		m_DaisyHAL->Delay(100); // TODO: maybe less?

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
		m_DaisyHAL->Delay(100);

		// TURN ON DISPLAY
		SendCommand(0x29);

		// MADCTL
		SendCommand(0x36);
		m_DaisyHAL->Delay(10);
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

		uint8 rotationBits = 0;

		switch (Orientation)
		{
		case Orientations::Upright:
		{
			m_RotatedWidth = Width;
			m_RotatedHeight = Height;
			rotationBits = ili_mx | ili_my | ili_mv | ili_bgr;
		}
		break;

		case Orientations::UpsideDown:
		{
			m_RotatedWidth = Height;
			m_RotatedHeight = Width;
			rotationBits = ili_my | ili_bgr;
		}
		break;

		case Orientations::ToRight:
		{
			m_RotatedWidth = Width;
			m_RotatedHeight = Height;
			rotationBits = ili_mx | ili_my | ili_mv | ili_bgr;
		}
		break;

		case Orientations::ToLeft:
		{
			m_RotatedWidth = Width;
			m_RotatedHeight = Height;
			rotationBits = ili_mv | ili_bgr;
		}
		break;
		}

		return rotationBits;
	}

	void SendCommand(uint8 cmd)
	{
		m_DC.Write(0);

		m_SPI.BlockingTransmit(&cmd, 1);
	}

	void SendData(uint8 *buff, uint32 size)
	{
		m_DC.Write(1);

		m_SPI.BlockingTransmit(buff, size);
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

	void PaintPixel(uint16 X, uint16 Y, uint16 R5G6B5, uint8 Alpha)
	{
		uint32 index = X + (Y * m_RotatedWidth);

		if (Alpha != 255)
		{
			uint16 currentColor = m_ColorBuffer[index];
			R5G6B5 = BlendR5G6B5(R5G6B5, currentColor, Alpha);
		}

		m_ColorBuffer[index] = R5G6B5;

		index *= 2;
		m_FrameBuffer[index] = R5G6B5 >> 8;
		m_FrameBuffer[index + 1] = R5G6B5 & 0xFF;

		// TODO: Lets divide the whole screen in 10 sectors, 32 pixel high each
		// uint8 screen_sector     = y / 32;
		// dirty_buff[screen_sector] = 1;
	}

	void Reset(void)
	{
		m_RST.Write(0);
		m_DaisyHAL->Delay(100);

		m_RST.Write(1);
		m_DaisyHAL->Delay(100);
	}

	void SendTheEntireDataDMA(void)
	{
		m_RemainingBufferLengthToSend = FRAME_BUFFER_LENGTH;

		SendDataDMA(m_FrameBuffer, BUFFER_CHUNK_SIZE);
	}

	void SendDataDMA(uint8 *Buffer, uint32 Length)
	{
		m_IsDMABusy = true;

		// Below line might help if there is draw artifacts
		dsy_dma_clear_cache_for_buffer(Buffer, Length);

		m_DC.Write(1);

		m_SPI.DmaTransmit(Buffer, Length, nullptr, &OnDMATransmissionCompleted, this);
	}

	static void OnDMATransmissionCompleted(void *Context, daisy::SpiHandle::Result Result)
	{
		if (Result != daisy::SpiHandle::Result::OK)
			return;

		ILI9341_HAL *thisPtr = static_cast<ILI9341_HAL *>(Context);

		uint32 transferSize = Math::Min(thisPtr->m_RemainingBufferLengthToSend, BUFFER_CHUNK_SIZE);
		// uint32 transferSize = thisPtr->m_RemainingBufferLengthToSend < 2 * BUFFER_CHUNK_SIZE ? thisPtr->m_RemainingBufferLengthToSend : 2 * BUFFER_CHUNK_SIZE;

		daisy::DaisySeed::PrintLine("%i %i", thisPtr->m_RemainingBufferLengthToSend, transferSize);

		thisPtr->m_RemainingBufferLengthToSend -= transferSize;
		if (thisPtr->m_RemainingBufferLengthToSend == 0)
		{
			thisPtr->m_IsDMABusy = false;
			return;
		}

		uint8 *data = thisPtr->m_FrameBuffer + (FRAME_BUFFER_LENGTH - thisPtr->m_RemainingBufferLengthToSend);
		thisPtr->SendDataDMA(data, transferSize);

		// 16bit transfer
		// thisPtr->SendDataDMA(&m_FrameBuffer[2 * BUFFER_CHUNK_SIZE], BUFFER_CHUNK_SIZE);
	}

	static uint16 BlendR5G6B5(uint16 ColorA, uint16 ColorB, uint8 Alpha)
	{
		//   rrrrrggggggbbbbb
		const uint16 MASK_RB = 63519;		// 0b1111100000011111
		const uint16 MASK_G = 2016;			// 0b0000011111100000
		const uint32 MASK_MUL_RB = 4065216; // 0b1111100000011111000000
		const uint32 MASK_MUL_G = 129024;	// 0b0000011111100000000000
		const uint16 MAX_ALPHA = 64;		// 6bits+1 with rounding

		// alpha for foreground multiplication
		// convert from 8bit to (6bit+1) with rounding
		// will be in [0..64] inclusive
		Alpha = (Alpha + 2) >> 2;
		// "beta" for background multiplication; (6bit+1);
		// will be in [0..64] inclusive
		uint8 beta = MAX_ALPHA - Alpha;
		// so (0..64)*alpha + (0..64)*beta always in 0..64

		return (uint16)((((Alpha * (uint32)(ColorA & MASK_RB) + beta * (uint32)(ColorB & MASK_RB)) & MASK_MUL_RB) | ((Alpha * (ColorA & MASK_G) + beta * (ColorB & MASK_G)) & MASK_MUL_G)) >> 6);
	}

private:
	DaisySeedHAL *m_DaisyHAL;
	GPIOPins m_PinSCLK, m_PinMOSI, m_PinNSS, m_PinDC, m_PinRST;
	Orientations m_Orientation;

	uint8 *m_ColorBuffer;
	// TODO: Maybe uint16* instead of uint8
	uint8 *m_FrameBuffer;

	daisy::SpiHandle m_SPI;

	daisy::GPIO m_RST;
	daisy::GPIO m_DC;
	daisy::GPIO m_CS;

	float m_TargetFrameRate;
	uint16 m_RotatedWidth;
	uint16 m_RotatedHeight;

	float m_NextUpdateTime;
	bool m_IsDMABusy;
	uint32 m_RemainingBufferLengthToSend;

	static const uint32 COLOR_BUFFER_LENGTH = Width * Height;
	static const uint32 FRAME_BUFFER_LENGTH = COLOR_BUFFER_LENGTH * 2;
	static const uint16 BUFFER_CHUNK_SIZE = FRAME_BUFFER_LENGTH / 3; // 8bit data
																	 // const uint16 BUFFER_CHUNK_SIZE = FRAME_BUFFER_LENGTH / 4; // 16bit data
};

typedef ILI9341_HAL<320, 240> ILI9341_HAL_320_240;