#ifdef ON_WINDOWS

#include "DaisySeedFramework/SDL_HAL.h"
#include <DigitalSignalProcessing/Math.h>
#include <DigitalSignalProcessing/Debug.h>
#include <DigitalSignalProcessing/Memory.h>

#undef ns
#undef ms
#include <SDL3/SDL.h>
#undef ns
#undef ms
//#define ns *0.000'001
//#define ms *0.001

#pragma comment(lib, "SDL3.lib")

SDL_HAL::SDL_HAL(IHAL* HAL, GPIOPins, GPIOPins, GPIOPins, GPIOPins, GPIOPins, Orientations Orientation, Point PixelSize)
	:m_HAL(HAL),
	m_Window(nullptr),
	m_Renderer(nullptr),
	m_Texture(nullptr),
	m_FrameBuffer(nullptr),
	m_TargetFrameRate(0),
	m_UpdateStep(0),
	m_NextUpdateTime(0)
{
	ASSERT(m_HAL != nullptr, "m_HAL cannot be null");
	ASSERT(PixelSize.X != 0, "PixelSize.X must be greater than zero");
	ASSERT(PixelSize.Y != 0, "PixelSize.Y must be greater than zero");

	if (Orientation == Orientations::UpRight || Orientation == Orientations::UpsideDown)
		m_Dimension = Point(PixelSize.Y, PixelSize.X);
	else
		m_Dimension = Point(PixelSize.X, PixelSize.Y);
}

void SDL_HAL::Initialize(void)
{
	ASSERT(SDL_Init(SDL_INIT_VIDEO), "Failed to initialize SDL");
	m_Window = SDL_CreateWindow("Screen", m_Dimension.X, m_Dimension.Y, SDL_WINDOW_OPENGL | SDL_WINDOW_ALWAYS_ON_TOP);
	m_Renderer = SDL_CreateRenderer(m_Window, nullptr);
	m_Texture = SDL_CreateTexture(m_Renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, m_Dimension.X, m_Dimension.Y);

	m_FrameBuffer = Memory::Allocate<uint16>(m_Dimension.X * m_Dimension.Y, true);
	Clear(ColorBlack);

	SetTargetFrameRate(MAX_FRAME_RATE);
}

void SDL_HAL::Update(void)
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

void SDL_HAL::SetTargetFrameRate(uint8 Value)
{
	ASSERT(Value != 0, "Invalid Value %f", Value);

	m_TargetFrameRate = Math::Min(MAX_FRAME_RATE, Value);

	m_UpdateStep = 1000 / m_TargetFrameRate;
}

void SDL_HAL::Clear(Color Color)
{
	uint16 color = Color.R5G6B5();

	for (uint32 y = 0; y < m_Dimension.Y; ++y)
		for (uint32 x = 0; x < m_Dimension.X; ++x)
			m_FrameBuffer[x + (y * m_Dimension.X)] = color;
}

void SDL_HAL::DrawPixel(Point Position, Color Color)
{
	if (Position.X < 0 || Position.Y < 0 ||
		Position.X >= m_Dimension.X || Position.Y >= m_Dimension.Y)
		return;

	PaintPixel(Position.X, Position.Y, Color.R5G6B5(), Color.A);
}

bool SDL_HAL::GetKeyState(uint16 Key) const
{
	const bool* keysState = SDL_GetKeyboardState(nullptr);

	return keysState[Key];
}

void SDL_HAL::PaintPixel(int16 X, int16 Y, uint16 R5G6B5, uint8 Alpha)
{
	uint32 index = X + (Y * m_Dimension.X);

	if (Alpha != 255)
	{
		uint16 currentColor = m_FrameBuffer[index];
		R5G6B5 = Color::BlendR5G6B5(R5G6B5, currentColor, Alpha);
	}

	m_FrameBuffer[index] = R5G6B5;
}

#endif