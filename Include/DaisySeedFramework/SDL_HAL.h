#pragma once
#ifndef SDL_HAL_H
#define SDL_HAL_H

#include "Common.h"
#include "I_LCD_HAL.h"
#include <DigitalSignalProcessing/IHAL.h>
#include <DigitalSignalProcessing/FixedFunction.h>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

class SDL_HAL : public I_LCD_HAL
{
	static constexpr uint8_t MAX_FRAME_RATE = 60;

public:
	typedef FixedFunction<void(void)> RenderEventHandler;

public:
	SDL_HAL(IHAL* HAL, GPIOPins, GPIOPins, GPIOPins, GPIOPins, GPIOPins, Orientations Orientation, Point PixelSize);

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

	bool GetKeyState(uint16_t Key) const;

private:
	void PaintPixel(int16_t X, int16_t Y, uint16_t R5G6B5, uint8_t Alpha);

private:
	IHAL* m_HAL;
	SDL_Window* m_Window;
	SDL_Renderer* m_Renderer;
	SDL_Texture* m_Texture;

	RenderEventHandler m_RenderListener;

	uint16_t* m_FrameBuffer;

	uint8_t m_TargetFrameRate;
	Point m_Dimension;

	uint16_t m_UpdateStep;
	uint32_t m_NextUpdateTime;
};

#endif