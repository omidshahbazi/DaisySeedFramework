#pragma once
#ifndef SDL_HAL_H
#define SDL_HAL_H

#include "I_LCD_HAL.h"
#include "DSP/Math.h"
#include "DSP/ContextCallback.h"

#undef ns
#undef ms
#include <SDL3/SDL.h>
#undef ns
#undef ms
#define ns *0.000'001
#define ms *0.001

#pragma comment(lib, "SDL3.lib")

template <uint32 Width, uint32 Height>
class SDL_HAL : public I_LCD_HAL
{
	static_assert(Width != 0, "Width must be greater than zero");
	static_assert(Height != 0, "Height must be greater than zero");

	static constexpr uint8 MAX_FRAME_RATE = 60;
	static constexpr uint32 FRAME_BUFFER_LENGTH = Width * Height;

public:
	typedef ContextCallback<void> RenderEventHandler;

public:
	SDL_HAL(IHAL* HAL, GPIOPins, GPIOPins, GPIOPins, GPIOPins, GPIOPins, Orientations Orientation)
		:m_HAL(HAL),
		m_Window(nullptr),
		m_Renderer(nullptr),
		m_Texture(nullptr),
		m_Orientation(Orientation),
		m_FrameBuffer(nullptr),
		m_TargetFrameRate(0),
		m_UpdateStep(0),
		m_NextUpdateTime(0)
	{
		if (Orientation == Orientations::Upright || Orientation == Orientations::UpsideDown)
			m_Dimension = Point(Height, Width);
		else
			m_Dimension = Point(Width, Height);
	}

	void Initialize(void)
	{
		ASSERT(SDL_Init(SDL_INIT_VIDEO), "Failed to initialize SDL");
		m_Window = SDL_CreateWindow("Screen", m_Dimension.X, m_Dimension.Y, SDL_WINDOW_OPENGL);
		m_Renderer = SDL_CreateRenderer(m_Window, nullptr);
		m_Texture = SDL_CreateTexture(m_Renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, m_Dimension.X, m_Dimension.Y);

		m_FrameBuffer = Memory::Allocate<uint16>(FRAME_BUFFER_LENGTH, true);
		Clear(ColorBlack);

		SetTargetFrameRate(MAX_FRAME_RATE);
	}

	void SetOnRender(RenderEventHandler Listener)
	{
		m_RenderListener = Listener;
	}

	void Update(void) override
	{
		SDL_Event event;
		while (SDL_PollEvent(&event));

		uint32 time = m_HAL->GetTimeSinceStartupMs();
		if (time < m_NextUpdateTime)
			return;
		m_NextUpdateTime = time + m_UpdateStep;

		SDL_RenderClear(m_Renderer);

		m_RenderListener();

		SDL_UpdateTexture(m_Texture, nullptr, m_FrameBuffer, m_Dimension.X * sizeof(uint16));
		SDL_RenderTexture(m_Renderer, m_Texture, NULL, NULL);
		SDL_RenderPresent(m_Renderer);
	}

	void SetTargetFrameRate(uint8 Value)
	{
		ASSERT(Value != 0, "Invalid Value %f", Value);

		m_TargetFrameRate = Math::Min(MAX_FRAME_RATE, Value);

		m_UpdateStep = 1000 / m_TargetFrameRate;
	}

	uint8 GetTargetFrameRate(void) const
	{
		return m_TargetFrameRate;
	}

	void Clear(Color Color) override
	{
		uint16 color = Color.R5G6B5();

		for (uint32 y = 0; y < m_Dimension.Y; ++y)
			for (uint32 x = 0; x < m_Dimension.X; ++x)
				m_FrameBuffer[x + (y * m_Dimension.X)] = color;
	}

	void DrawPixel(Point Position, Color Color) override
	{
		if (Position.X < 0 || Position.Y < 0 ||
			Position.X >= m_Dimension.X || Position.Y >= m_Dimension.Y)
			return;

		PaintPixel(Position.X, Position.Y, Color.R5G6B5(), Color.A);
	}

	const Point& GetDimension(void) const override
	{
		return m_Dimension;
	}

	bool GetKeyState(uint16 Key) const
	{
		const bool* keysState = SDL_GetKeyboardState(nullptr);

		return keysState[Key];
	}

private:
	void PaintPixel(int16 X, int16 Y, uint16 R5G6B5, uint8 Alpha)
	{
		uint32 index = X + (Y * m_Dimension.X);

		if (Alpha != 255)
		{
			uint16 currentColor = m_FrameBuffer[index];
			R5G6B5 = Color::BlendR5G6B5(R5G6B5, currentColor, Alpha);
		}

		m_FrameBuffer[index] = R5G6B5;
	}

private:
	IHAL* m_HAL;
	SDL_Window* m_Window;
	SDL_Renderer* m_Renderer;
	SDL_Texture* m_Texture;

	Orientations m_Orientation;

	RenderEventHandler m_RenderListener;

	uint16* m_FrameBuffer;

	uint8 m_TargetFrameRate;
	Point m_Dimension;

	uint16 m_UpdateStep;
	uint32 m_NextUpdateTime;
};

#endif