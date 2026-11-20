#ifdef ON_WINDOWS

#define USE_SOFTWARE_RENDERER

#include "DaisySeedFramework/SDL_HAL.h"
#include "DaisySeedFramework/WindowsInclude.h"
#include <DigitalSignalProcessing/Math.h>
#include <DigitalSignalProcessing/Debug.h>
#include <DigitalSignalProcessing/Memory.h>

#pragma comment(lib, "SDL3-static.lib")
#pragma comment(lib, "imm32.lib")
#pragma comment(lib, "version.lib")

static uint16_t MapToVirtualKey(KeyboardKeys Key)
{
	switch (Key)
	{
	case KeyboardKeys::A: return 'A';
	case KeyboardKeys::B: return 'B';
	case KeyboardKeys::C: return 'C';
	case KeyboardKeys::D: return 'D';
	case KeyboardKeys::E: return 'E';
	case KeyboardKeys::F: return 'F';
	case KeyboardKeys::G: return 'G';
	case KeyboardKeys::H: return 'H';
	case KeyboardKeys::I: return 'I';
	case KeyboardKeys::J: return 'J';
	case KeyboardKeys::K: return 'K';
	case KeyboardKeys::L: return 'L';
	case KeyboardKeys::M: return 'M';
	case KeyboardKeys::N: return 'N';
	case KeyboardKeys::O: return 'O';
	case KeyboardKeys::P: return 'P';
	case KeyboardKeys::Q: return 'Q';
	case KeyboardKeys::R: return 'R';
	case KeyboardKeys::S: return 'S';
	case KeyboardKeys::T: return 'T';
	case KeyboardKeys::U: return 'U';
	case KeyboardKeys::V: return 'V';
	case KeyboardKeys::W: return 'W';
	case KeyboardKeys::X: return 'X';
	case KeyboardKeys::Y: return 'Y';
	case KeyboardKeys::Z: return 'Z';

	case KeyboardKeys::Key1: return '1';
	case KeyboardKeys::Key2: return '2';
	case KeyboardKeys::Key3: return '3';
	case KeyboardKeys::Key4: return '4';
	case KeyboardKeys::Key5: return '5';
	case KeyboardKeys::Key6: return '6';
	case KeyboardKeys::Key7: return '7';
	case KeyboardKeys::Key8: return '8';
	case KeyboardKeys::Key9: return '9';
	case KeyboardKeys::Key0: return '0';

	case KeyboardKeys::Return:    return VK_RETURN;
	case KeyboardKeys::Escape:    return VK_ESCAPE;
	case KeyboardKeys::Backspace: return VK_BACK;
	case KeyboardKeys::Tab:       return VK_TAB;
	case KeyboardKeys::Space:     return VK_SPACE;

	case KeyboardKeys::Minus:        return VK_OEM_MINUS;
	case KeyboardKeys::Equals:       return VK_OEM_PLUS;
	case KeyboardKeys::LeftBracket:  return VK_OEM_4;
	case KeyboardKeys::RightBracket: return VK_OEM_6;
	case KeyboardKeys::BackSlash:    return VK_OEM_5;

	case KeyboardKeys::SemiColon:  return VK_OEM_1;
	case KeyboardKeys::Apostrophe: return VK_OEM_7;
	case KeyboardKeys::Grave:      return VK_OEM_3;

	case KeyboardKeys::Comma:  return VK_OEM_COMMA;
	case KeyboardKeys::Period: return VK_OEM_PERIOD;
	case KeyboardKeys::Slash:  return VK_OEM_2;

	case KeyboardKeys::CapsLock: return VK_CAPITAL;

	case KeyboardKeys::F1:  return VK_F1;
	case KeyboardKeys::F2:  return VK_F2;
	case KeyboardKeys::F3:  return VK_F3;
	case KeyboardKeys::F4:  return VK_F4;
	case KeyboardKeys::F5:  return VK_F5;
	case KeyboardKeys::F6:  return VK_F6;
	case KeyboardKeys::F7:  return VK_F7;
	case KeyboardKeys::F8:  return VK_F8;
	case KeyboardKeys::F9:  return VK_F9;
	case KeyboardKeys::F10: return VK_F10;
	case KeyboardKeys::F11: return VK_F11;
	case KeyboardKeys::F12: return VK_F12;

	case KeyboardKeys::PrintScreen: return VK_SNAPSHOT;
	case KeyboardKeys::ScrollLock:  return VK_SCROLL;
	case KeyboardKeys::Pause:       return VK_PAUSE;
	case KeyboardKeys::Insert:      return VK_INSERT;

	case KeyboardKeys::Home:     return VK_HOME;
	case KeyboardKeys::PageUp:   return VK_PRIOR;
	case KeyboardKeys::Delete:   return VK_DELETE;
	case KeyboardKeys::End:      return VK_END;
	case KeyboardKeys::PageDown: return VK_NEXT;
	case KeyboardKeys::Right:    return VK_RIGHT;
	case KeyboardKeys::Left:     return VK_LEFT;
	case KeyboardKeys::Down:     return VK_DOWN;
	case KeyboardKeys::Up:       return VK_UP;

	case KeyboardKeys::Numlock: return VK_NUMLOCK;

	case KeyboardKeys::KeyPadDivide:   return VK_DIVIDE;
	case KeyboardKeys::KeyPadMultiply: return VK_MULTIPLY;
	case KeyboardKeys::KeyPadMinus:    return VK_SUBTRACT;
	case KeyboardKeys::KeyPadPlus:     return VK_ADD;
	case KeyboardKeys::KeyPadEnter:    return VK_RETURN; // no separate VK code
	case KeyboardKeys::KeyPad1:        return VK_NUMPAD1;
	case KeyboardKeys::KeyPad2:        return VK_NUMPAD2;
	case KeyboardKeys::KeyPad3:        return VK_NUMPAD3;
	case KeyboardKeys::KeyPad4:        return VK_NUMPAD4;
	case KeyboardKeys::KeyPad5:        return VK_NUMPAD5;
	case KeyboardKeys::KeyPad6:        return VK_NUMPAD6;
	case KeyboardKeys::KeyPad7:        return VK_NUMPAD7;
	case KeyboardKeys::KeyPad8:        return VK_NUMPAD8;
	case KeyboardKeys::KeyPad9:        return VK_NUMPAD9;
	case KeyboardKeys::KeyPad0:        return VK_NUMPAD0;
	case KeyboardKeys::KeyPadPeriod:   return VK_DECIMAL;

	case KeyboardKeys::LeftControl:  return VK_LCONTROL;
	case KeyboardKeys::LeftShift:    return VK_LSHIFT;
	case KeyboardKeys::LeftAlt:      return VK_LMENU;
	case KeyboardKeys::RightControl: return VK_RCONTROL;
	case KeyboardKeys::RightShift:   return VK_RSHIFT;
	case KeyboardKeys::RightAlt:     return VK_RMENU;
	}

	NOT_IMPLEMENTED();
}

static uint16_t MapToSDLScanCode(KeyboardKeys Key)
{
	switch (Key)
	{
	case KeyboardKeys::A: return SDL_SCANCODE_A;
	case KeyboardKeys::B: return SDL_SCANCODE_B;
	case KeyboardKeys::C: return SDL_SCANCODE_C;
	case KeyboardKeys::D: return SDL_SCANCODE_D;
	case KeyboardKeys::E: return SDL_SCANCODE_E;
	case KeyboardKeys::F: return SDL_SCANCODE_F;
	case KeyboardKeys::G: return SDL_SCANCODE_G;
	case KeyboardKeys::H: return SDL_SCANCODE_H;
	case KeyboardKeys::I: return SDL_SCANCODE_I;
	case KeyboardKeys::J: return SDL_SCANCODE_J;
	case KeyboardKeys::K: return SDL_SCANCODE_K;
	case KeyboardKeys::L: return SDL_SCANCODE_L;
	case KeyboardKeys::M: return SDL_SCANCODE_M;
	case KeyboardKeys::N: return SDL_SCANCODE_N;
	case KeyboardKeys::O: return SDL_SCANCODE_O;
	case KeyboardKeys::P: return SDL_SCANCODE_P;
	case KeyboardKeys::Q: return SDL_SCANCODE_Q;
	case KeyboardKeys::R: return SDL_SCANCODE_R;
	case KeyboardKeys::S: return SDL_SCANCODE_S;
	case KeyboardKeys::T: return SDL_SCANCODE_T;
	case KeyboardKeys::U: return SDL_SCANCODE_U;
	case KeyboardKeys::V: return SDL_SCANCODE_V;
	case KeyboardKeys::W: return SDL_SCANCODE_W;
	case KeyboardKeys::X: return SDL_SCANCODE_X;
	case KeyboardKeys::Y: return SDL_SCANCODE_Y;
	case KeyboardKeys::Z: return SDL_SCANCODE_Z;

	case KeyboardKeys::Key1: return SDL_SCANCODE_1;
	case KeyboardKeys::Key2: return SDL_SCANCODE_2;
	case KeyboardKeys::Key3: return SDL_SCANCODE_3;
	case KeyboardKeys::Key4: return SDL_SCANCODE_4;
	case KeyboardKeys::Key5: return SDL_SCANCODE_5;
	case KeyboardKeys::Key6: return SDL_SCANCODE_6;
	case KeyboardKeys::Key7: return SDL_SCANCODE_7;
	case KeyboardKeys::Key8: return SDL_SCANCODE_8;
	case KeyboardKeys::Key9: return SDL_SCANCODE_9;
	case KeyboardKeys::Key0: return SDL_SCANCODE_0;

	case KeyboardKeys::Return:    return SDL_SCANCODE_RETURN;
	case KeyboardKeys::Escape:    return SDL_SCANCODE_ESCAPE;
	case KeyboardKeys::Backspace: return SDL_SCANCODE_BACKSPACE;
	case KeyboardKeys::Tab:       return SDL_SCANCODE_TAB;
	case KeyboardKeys::Space:     return SDL_SCANCODE_SPACE;

	case KeyboardKeys::Minus:        return SDL_SCANCODE_MINUS;
	case KeyboardKeys::Equals:       return SDL_SCANCODE_EQUALS;
	case KeyboardKeys::LeftBracket:  return SDL_SCANCODE_LEFTBRACKET;
	case KeyboardKeys::RightBracket: return SDL_SCANCODE_RIGHTBRACKET;
	case KeyboardKeys::BackSlash:    return SDL_SCANCODE_BACKSLASH;

	case KeyboardKeys::SemiColon:  return SDL_SCANCODE_SEMICOLON;
	case KeyboardKeys::Apostrophe: return SDL_SCANCODE_APOSTROPHE;
	case KeyboardKeys::Grave:      return SDL_SCANCODE_GRAVE;

	case KeyboardKeys::Comma:  return SDL_SCANCODE_COMMA;
	case KeyboardKeys::Period: return SDL_SCANCODE_PERIOD;
	case KeyboardKeys::Slash:  return SDL_SCANCODE_SLASH;

	case KeyboardKeys::CapsLock: return SDL_SCANCODE_CAPSLOCK;

	case KeyboardKeys::F1:  return SDL_SCANCODE_F1;
	case KeyboardKeys::F2:  return SDL_SCANCODE_F2;
	case KeyboardKeys::F3:  return SDL_SCANCODE_F3;
	case KeyboardKeys::F4:  return SDL_SCANCODE_F4;
	case KeyboardKeys::F5:  return SDL_SCANCODE_F5;
	case KeyboardKeys::F6:  return SDL_SCANCODE_F6;
	case KeyboardKeys::F7:  return SDL_SCANCODE_F7;
	case KeyboardKeys::F8:  return SDL_SCANCODE_F8;
	case KeyboardKeys::F9:  return SDL_SCANCODE_F9;
	case KeyboardKeys::F10: return SDL_SCANCODE_F10;
	case KeyboardKeys::F11: return SDL_SCANCODE_F11;
	case KeyboardKeys::F12: return SDL_SCANCODE_F12;

	case KeyboardKeys::PrintScreen: return SDL_SCANCODE_PRINTSCREEN;
	case KeyboardKeys::ScrollLock:  return SDL_SCANCODE_SCROLLLOCK;
	case KeyboardKeys::Pause:       return SDL_SCANCODE_PAUSE;
	case KeyboardKeys::Insert:      return SDL_SCANCODE_INSERT;

	case KeyboardKeys::Home:     return SDL_SCANCODE_HOME;
	case KeyboardKeys::PageUp:   return SDL_SCANCODE_PAGEUP;
	case KeyboardKeys::Delete:   return SDL_SCANCODE_DELETE;
	case KeyboardKeys::End:      return SDL_SCANCODE_END;
	case KeyboardKeys::PageDown: return SDL_SCANCODE_PAGEDOWN;
	case KeyboardKeys::Right:    return SDL_SCANCODE_RIGHT;
	case KeyboardKeys::Left:     return SDL_SCANCODE_LEFT;
	case KeyboardKeys::Down:     return SDL_SCANCODE_DOWN;
	case KeyboardKeys::Up:       return SDL_SCANCODE_UP;

	case KeyboardKeys::Numlock: return SDL_SCANCODE_NUMLOCKCLEAR;

	case KeyboardKeys::KeyPadDivide:   return SDL_SCANCODE_KP_DIVIDE;
	case KeyboardKeys::KeyPadMultiply: return SDL_SCANCODE_KP_MULTIPLY;
	case KeyboardKeys::KeyPadMinus:    return SDL_SCANCODE_KP_MINUS;
	case KeyboardKeys::KeyPadPlus:     return SDL_SCANCODE_KP_PLUS;
	case KeyboardKeys::KeyPadEnter:    return SDL_SCANCODE_KP_ENTER;
	case KeyboardKeys::KeyPad1:        return SDL_SCANCODE_KP_1;
	case KeyboardKeys::KeyPad2:        return SDL_SCANCODE_KP_2;
	case KeyboardKeys::KeyPad3:        return SDL_SCANCODE_KP_3;
	case KeyboardKeys::KeyPad4:        return SDL_SCANCODE_KP_4;
	case KeyboardKeys::KeyPad5:        return SDL_SCANCODE_KP_5;
	case KeyboardKeys::KeyPad6:        return SDL_SCANCODE_KP_6;
	case KeyboardKeys::KeyPad7:        return SDL_SCANCODE_KP_7;
	case KeyboardKeys::KeyPad8:        return SDL_SCANCODE_KP_8;
	case KeyboardKeys::KeyPad9:        return SDL_SCANCODE_KP_9;
	case KeyboardKeys::KeyPad0:        return SDL_SCANCODE_KP_0;
	case KeyboardKeys::KeyPadPeriod:   return SDL_SCANCODE_KP_PERIOD;

	case KeyboardKeys::LeftControl:  return SDL_SCANCODE_LCTRL;
	case KeyboardKeys::LeftShift:    return SDL_SCANCODE_LSHIFT;
	case KeyboardKeys::LeftAlt:      return SDL_SCANCODE_LALT;
	case KeyboardKeys::RightControl: return SDL_SCANCODE_RCTRL;
	case KeyboardKeys::RightShift:   return SDL_SCANCODE_RSHIFT;
	case KeyboardKeys::RightAlt:     return SDL_SCANCODE_RALT;
	}

	NOT_IMPLEMENTED();
}

#define CHECK_CALL(Expr) \
	{ \
		bool result = Expr; \
		ASSERT(result, #Expr); \
	}

static cstr CLASS_NAME = "SDL_HAL_CLASS";

static LRESULT CALLBACK ChildProc(HWND h, UINT msg, WPARAM wp, LPARAM lp)
{
	switch (msg)
	{
	case WM_LBUTTONDOWN:
		SetFocus(h);
		return 0;
	}

	return DefWindowProc(h, msg, wp, lp);
}

SDL_HAL::SDL_HAL(NotNull<IHAL> HAL, GPIOPins, GPIOPins, GPIOPins, GPIOPins, GPIOPins, Orientations Orientation, Point PixelSize)
	:m_HAL(HAL),
	m_WindowHandle(nullptr),
	m_Window(nullptr),
	m_Renderer(nullptr),
	m_Texture(nullptr),
	m_FrameBuffer(nullptr),
	m_TargetFrameRate(0),
	m_IsQuitTriggered(false),
	m_UpdateStep(0),
	m_NextUpdateTime(0)
{
	ASSERT(PixelSize.X != 0, "PixelSize.X must be greater than zero");
	ASSERT(PixelSize.Y != 0, "PixelSize.Y must be greater than zero");

	if (Orientation == Orientations::UpRight || Orientation == Orientations::UpsideDown)
		m_Dimension = Point(PixelSize.Y, PixelSize.X);
	else
		m_Dimension = Point(PixelSize.X, PixelSize.Y);
}

bool SDL_HAL::Initialize(bool AsyncKeyCapture, WindowHandle Parent)
{
	ASSERT(m_Window == nullptr, "SDL_HAL is already initialized");

	CHECK_CALL(SDL_Init(SDL_INIT_VIDEO));

	SDL_PropertiesID props = SDL_CreateProperties();
	CHECK_CALL(SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Screen"));
	CHECK_CALL(SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, m_Dimension.X));
	CHECK_CALL(SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, m_Dimension.Y));
#ifndef USE_SOFTWARE_RENDERER
	SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);
#endif

	if (Parent != nullptr)
	{
		HINSTANCE module = GetModuleHandle(nullptr);

		static bool registered = false;
		if (!registered)
		{
			WNDCLASSEX wc = {};
			wc.cbSize = sizeof(wc);
			wc.style = CS_OWNDC;
			wc.lpfnWndProc = ChildProc;
			wc.hInstance = module;
			wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
			wc.lpszClassName = CLASS_NAME;
			CHECK_CALL(RegisterClassEx(&wc));

			registered = true;
		}

		m_WindowHandle = CreateWindowEx(0, CLASS_NAME, "", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, m_Dimension.X, m_Dimension.Y, (HWND)Parent, nullptr, module, nullptr);
		ASSERT(m_WindowHandle != nullptr, "Couldn't create native window");

		SetWindowLongPtr((HWND)m_WindowHandle, GWLP_USERDATA, (LONG_PTR)this);

		CHECK_CALL(SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, m_WindowHandle));
	}

	m_Window = SDL_CreateWindowWithProperties(props);
	ASSERT(m_Window != nullptr, "Failed to create window");

	HDC dc = GetDC((HWND)m_WindowHandle);
	HBRUSH b = CreateSolidBrush(RGB(0, 255, 0));
	RECT r = { 0, 0, m_Dimension.X, m_Dimension.Y };
	FillRect(dc, &r, b);
	DeleteObject(b);
	ReleaseDC((HWND)m_WindowHandle, dc);

	SDL_DestroyProperties(props);

#ifdef USE_SOFTWARE_RENDERER
	m_Renderer = SDL_CreateRenderer(m_Window, "software");
#else
	m_Renderer = SDL_CreateRenderer(m_Window, nullptr);
#endif
	ASSERT_NOT_NULL(m_Renderer);

	m_Texture = SDL_CreateTexture(m_Renderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, m_Dimension.X, m_Dimension.Y);
	ASSERT_NOT_NULL(m_Texture);

	m_FrameBuffer = Memory::Allocate<uint16_t>(m_Dimension.X * m_Dimension.Y, true);
	Clear(ColorBlack);

	SetTargetFrameRate(MAX_FRAME_RATE);

	m_AsyncKeyCapture = AsyncKeyCapture;

	return true;
}

bool SDL_HAL::Deinitialize(void)
{
	ASSERT(m_Window != nullptr, "SDL_HAL is not initialized");

	Memory::Deallocate(m_FrameBuffer);
	m_FrameBuffer = nullptr;

	SDL_DestroyTexture(m_Texture);
	m_Texture = nullptr;

	SDL_DestroyRenderer(m_Renderer);
	m_Renderer = nullptr;

	SDL_DestroyWindow(m_Window);
	m_Window = nullptr;

	DestroyWindow((HWND)m_WindowHandle);

	SDL_Quit();

	m_IsQuitTriggered = false;

	return true;
}

void SDL_HAL::Update(bool Force)
{
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_EVENT_QUIT)
		{
			m_IsQuitTriggered = true;
			return;
		}
	}

	uint32_t time = m_HAL->GetTimeSinceStartupMs();
	if (!Force && time < m_NextUpdateTime)
		return;
	m_NextUpdateTime = time + m_UpdateStep;

	CHECK_CALL(SDL_RenderClear(m_Renderer));

	m_RenderListener();

	CHECK_CALL(SDL_UpdateTexture(m_Texture, nullptr, m_FrameBuffer, m_Dimension.X * sizeof(uint16_t)));
	CHECK_CALL(SDL_RenderTexture(m_Renderer, m_Texture, NULL, NULL));
	CHECK_CALL(SDL_RenderPresent(m_Renderer));
}

void SDL_HAL::SetTargetFrameRate(puint8_t Value)
{
	m_TargetFrameRate = Math::Min(MAX_FRAME_RATE, (uint8_t)Value);

	m_UpdateStep = 1000 / m_TargetFrameRate;
}

void SDL_HAL::Clear(Color Color)
{
	uint16_t color = Color.R5G6B5();

	for (uint32_t y = 0; y < m_Dimension.Y; ++y)
		for (uint32_t x = 0; x < m_Dimension.X; ++x)
			m_FrameBuffer[x + (y * m_Dimension.X)] = color;
}

void SDL_HAL::DrawPixel(Point Position, Color Color)
{
	if (Position.X < 0 || Position.Y < 0 ||
		Position.X >= m_Dimension.X || Position.Y >= m_Dimension.Y)
		return;

	PaintPixel(Position.X, Position.Y, Color.R5G6B5(), Color.A);
}

bool SDL_HAL::GetKeyState(KeyboardKeys Key) const
{
	if (m_AsyncKeyCapture)
		return (GetAsyncKeyState(MapToVirtualKey(Key)) & 0x8000) != 0;

	const bool* keysState = SDL_GetKeyboardState(nullptr);
	return keysState[MapToSDLScanCode(Key)];
}

void SDL_HAL::PaintPixel(int16_t X, int16_t Y, uint16_t R5G6B5, uint8_t Alpha)
{
	uint32_t index = X + (Y * m_Dimension.X);

	if (Alpha != 255)
	{
		uint16_t currentColor = m_FrameBuffer[index];
		R5G6B5 = Color::BlendR5G6B5(R5G6B5, currentColor, Alpha);
	}

	m_FrameBuffer[index] = R5G6B5;
}

#endif