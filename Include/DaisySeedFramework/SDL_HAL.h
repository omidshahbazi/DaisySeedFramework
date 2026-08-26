#pragma once
#ifndef SDL_HAL_H
#define SDL_HAL_H

#include "Common.h"
#include "I_LCD_HAL.h"
#include <DigitalSignalProcessing/IHAL.h>
#include <DigitalSignalProcessing/ContextCallback.h>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

class SDL_HAL : public I_LCD_HAL
{
	static constexpr uint8 MAX_FRAME_RATE = 60;

public:
	typedef ContextCallback<void> RenderEventHandler;

public:
	SDL_HAL(IHAL* HAL, GPIOPins, GPIOPins, GPIOPins, GPIOPins, GPIOPins, Orientations Orientation, Point PixelSize);

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

	bool GetKeyState(uint16 Key) const;

private:
	void PaintPixel(int16 X, int16 Y, uint16 R5G6B5, uint8 Alpha);

private:
	IHAL* m_HAL;
	SDL_Window* m_Window;
	SDL_Renderer* m_Renderer;
	SDL_Texture* m_Texture;

	RenderEventHandler m_RenderListener;

	uint16* m_FrameBuffer;

	uint8 m_TargetFrameRate;
	Point m_Dimension;

	uint16 m_UpdateStep;
	uint32 m_NextUpdateTime;
};

#endif