#pragma once
#ifndef VST3_HAL_H
#define VST3_HAL_H

#include "WindowsHAL.h"
#include "Version.h"
#include "Rect.h"
#include <DigitalSignalProcessing/FixedFunction.h>

class IPluginEvents
{
public:
	virtual void OnInitialize(void) = 0;
	virtual void OnTerminate(void) = 0;

	virtual void OnUpdate(void) = 0;
	virtual void OnProcessAudio(const float* const* In, float** Out, uint32_t Size) = 0;

	virtual Point GetViewSize(void) const = 0;
	virtual void OnSetViewEnabled(bool Enabled, void* ParentHandle = nullptr) = 0;
};

class VST3HAL : public WindowsHAL, private IPluginEvents
{
	friend class VST3HALPlugin;

protected:
	typedef FixedFunction<void(void)> DefaultCallback;
	typedef FixedFunction<Point(void)> GetViewSizeCallback;
	typedef FixedFunction<void(bool Enabled, void* ParentHandle)> SetViewEnabledCallback;

public:
	struct CreationInfo : public WindowsHAL::CreationInfo
	{};

	struct SetupInfo : public WindowsHAL::SetupInfo
	{
	public:
		cstr Vendor;
		cstr URL;
		cstr Email;
		uint32_t ID;
		cstr Name;
		Version Version;
		DefaultCallback OnInitialize;
		DefaultCallback OnTerminate;
		DefaultCallback OnUpdate;
		GetViewSizeCallback OnGetViewSize;
		SetViewEnabledCallback OnSetViewEnabled;
	};

public:
	VST3HAL(const CreationInfo& Info);

	void Setup(const SetupInfo& Info);
	bool TearDown(void) override;

	void StartAudio(AudioPassthrough Callback) override
	{
		m_AudioCallback = Callback;
	}

private:
	void OnInitialize(void) override
	{
		m_OnInitialize();
	}

	void OnTerminate(void) override
	{
		m_OnTerminate();
	}

	void OnUpdate(void) override
	{
		m_OnUpdate();
	}

	void OnProcessAudio(const float* const* In, float** Out, uint32_t Size) override;

	Point GetViewSize(void) const
	{
		return m_OnGetViewSize();
	}

	void OnSetViewEnabled(bool Enabled, void* ParentHandle = nullptr) override
	{
		m_OnSetViewEnabled(Enabled, ParentHandle);
	}

public:
	static bool InitializeModule(void);
	static bool TerminateModule(void);
	static void* GetFactory(void);

private:
	DefaultCallback m_OnInitialize;
	DefaultCallback m_OnTerminate;
	DefaultCallback m_OnUpdate;

	uint8_t m_FrameLength;
	AudioPassthrough m_AudioCallback;

	GetViewSizeCallback m_OnGetViewSize;
	SetViewEnabledCallback m_OnSetViewEnabled;
};

#define DEFINE_PLUGIN_ENTRY_POINT() \
	extern "C" \
	{ \
		__declspec(dllexport) bool InitDll(void) \
		{ \
			return VST3HAL::InitializeModule(); \
		} \
		__declspec(dllexport) bool ExitDll(void) \
		{ \
			return VST3HAL::TerminateModule(); \
		} \
		__declspec (dllexport) void* __stdcall GetPluginFactory() \
		{ \
			return VST3HAL::GetFactory(); \
		} \
	}

#endif