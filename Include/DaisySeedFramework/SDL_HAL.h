#pragma once
#ifndef SDL_HAL_H
#define SDL_HAL_H

#include "Common.h"
#include "I_LCD_HAL.h"
#include <DigitalSignalProcessing/IHAL.h>
#include <DigitalSignalProcessing/NotNull.h>
#include <DigitalSignalProcessing/FixedFunction.h>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

enum class KeyboardKeys
{
	A = 0,
	B,
	C,
	D,
	E,
	F,
	G,
	H,
	I,
	J,
	K,
	L,
	M,
	N,
	O,
	P,
	Q,
	R,
	S,
	T,
	U,
	V,
	W,
	X,
	Y,
	Z,

	Key1,
	Key2,
	Key3,
	Key4,
	Key5,
	Key6,
	Key7,
	Key8,
	Key9,
	Key0,

	Return,
	Escape,
	Backspace,
	Tab,
	Space,

	Minus,
	Equals,
	LeftBracket,
	RightBracket,
	BackSlash,

	SemiColon,
	Apostrophe,
	Grave,

	Comma,
	Period,
	Slash,

	CapsLock,

	F1,
	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,

	PrintScreen,
	ScrollLock,
	Pause,
	Insert,

	Home,
	PageUp,
	Delete,
	End,
	PageDown,
	Right,
	Left,
	Down,
	Up,

	Numlock,

	KeyPadDivide,
	KeyPadMultiply,
	KeyPadMinus,
	KeyPadPlus,
	KeyPadEnter,
	KeyPad1,
	KeyPad2,
	KeyPad3,
	KeyPad4,
	KeyPad5,
	KeyPad6,
	KeyPad7,
	KeyPad8,
	KeyPad9,
	KeyPad0,
	KeyPadPeriod,

	LeftControl,
	LeftShift,
	LeftAlt,
	RightControl,
	RightShift,
	RightAlt,

	COUNT
};

class SDL_HAL : public I_LCD_HAL
{
	static constexpr uint8_t MAX_FRAME_RATE = 60;

public:
	typedef void* WindowHandle;
	typedef FixedFunction<void(void)> RenderEventHandler;

public:
	SDL_HAL(NotNull<IHAL> HAL, GPIOPins, GPIOPins, GPIOPins, GPIOPins, GPIOPins, Orientations Orientation, Point PixelSize);

	bool Initialize(bool AsyncKeyCapture = false, WindowHandle Parent = nullptr);
	bool Deinitialize(void);

	void SetOnRender(RenderEventHandler Listener)
	{
		m_RenderListener = Listener;
	}

	void Update(bool Force = false) override;

	void SetTargetFrameRate(puint8_t Value);

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

	bool GetKeyState(KeyboardKeys Key) const;
	bool IsQuitTriggered(void) const
	{
		return m_IsQuitTriggered;
	}

private:
	void PaintPixel(int16_t X, int16_t Y, uint16_t R5G6B5, uint8_t Alpha);

private:
	IHAL* m_HAL;
	WindowHandle m_WindowHandle;
	SDL_Window* m_Window;
	SDL_Renderer* m_Renderer;
	SDL_Texture* m_Texture;

	RenderEventHandler m_RenderListener;

	uint16_t* m_FrameBuffer;

	uint8_t m_TargetFrameRate;
	Point m_Dimension;

	bool m_IsQuitTriggered;

	uint16_t m_UpdateStep;
	uint32_t m_NextUpdateTime;

	bool m_AsyncKeyCapture;
};

#endif