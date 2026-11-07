#pragma once
#ifndef VST3_HAL_H
#define VST3_HAL_H

#include "WindowsHAL.h"

namespace Steinberg
{
	class FUnknown;
	class IPluginFactory;
}

class VST3HALImplementation;

class VST3HAL : public WindowsHAL
{
public:
	VST3HAL(void* SDRAMAddress = nullptr, uint32_t SDRAMSize = 0);

	void Setup(uint8_t FrameLength, uint32_t SampleRate, bool Boost) override;

	static Steinberg::FUnknown* createInstance(void* context);

	static Steinberg::IPluginFactory* GetFactory(cstr Vendor, cstr URL, cstr Email, uint32_t ID, cstr Category, cstr Name);

private:
	VST3HALImplementation* m_Impl;
};

#define DEFINE_PLUGIN_ENTRY_POINT(Vendor, URL, Email, ID, Category, Name) \
	extern "C" \
	{ \
		__declspec (dllexport) Steinberg::IPluginFactory* __stdcall GetPluginFactory() \
		{ \
			return VST3HAL::GetFactory(Vendor, URL, Email, ID, Category, Name); \
		} \
	}

#endif