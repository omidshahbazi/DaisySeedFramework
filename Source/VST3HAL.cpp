#ifdef ON_WINDOWS

#include "DaisySeedFramework/VST3HAL.h"

#ifdef DEBUG
#define _DEBUG
#else
#define NDEBUG
#endif

#include <public.sdk/source/main/pluginfactory.h>
#include <public.sdk/source/vst/vstaudioeffect.h>
#include <public.sdk/source/vst/vsteditcontroller.h>

using namespace Steinberg;
using namespace Steinberg::Vst;

#pragma comment(lib, "sdk.lib")
#pragma comment(lib, "sdk_common.lib")
#pragma comment(lib, "base.lib")
#pragma comment(lib, "pluginterfaces.lib")

static const FUID kControllerUID(0xA1B2C3D4, 0x11223344, 0x55667788, 0x99AABBCC);

class VST3HALController : public EditController
{
public:
	static FUnknown* createInstance(void*)
	{
		return (IEditController*)new VST3HALController();
	}
};

class VST3HALImplementation : public AudioEffect
{
public:
	VST3HALImplementation(void)
	{
		setControllerClass(kControllerUID);
	}

	int32_t PLUGIN_API initialize(FUnknown* context) override
	{
		tresult result = AudioEffect::initialize(context);
		if (result != kResultOk)
			return result;

		if (result == kResultTrue)
		{
			addAudioInput(STR16("Stereo In"), SpeakerArr::kStereo);
			addAudioOutput(STR16("Stereo Out"), SpeakerArr::kStereo);
		}

		return kResultOk;
	}

	int32_t PLUGIN_API process(ProcessData& data) override
	{
		if (data.numInputs == 0 || data.numOutputs == 0 || data.numSamples == 0)
			return kResultOk;

		float** in = data.inputs[0].channelBuffers32;
		float** out = data.outputs[0].channelBuffers32;
		int32_t numSamples = data.numSamples;

		for (int32_t i = 0; i < data.inputs[0].numChannels; ++i)
		{
			float* channelData = reinterpret_cast<float*>(out[i]);

			//m_Distortion.Process(channelData, static_cast<uint8_t>(numSamples));
		}

		return kResultOk;
	}

	static FUnknown* createInstance(void* context);
};

VST3HAL::VST3HAL(void* SDRAMAddress, uint32_t SDRAMSize)
	: WindowsHAL(SDRAMAddress, SDRAMSize)
{}

void VST3HAL::Setup(uint8_t FrameLength, uint32_t SampleRate, bool Boost)
{
	//ASSERT(FrameLength != 0, "Invalid FrameLength %i", FrameLength);
}

FUnknown* VST3HAL::createInstance(void* context)
{
	return (IAudioProcessor*)new VST3HALImplementation();
}

IPluginFactory* VST3HAL::GetFactory(cstr Vendor, cstr URL, cstr Email, uint32_t ID, cstr Category, cstr Name)
{
	if (gPluginFactory == nullptr)
	{
		PFactoryInfo factoryInfo(Vendor, URL, Email, kDefaultFactoryFlags);
		gPluginFactory = new CPluginFactory(factoryInfo);

		static const FUID uid(ID, 0x4D415354, 0x45525321, 0x4C494E31);
		static PClassInfo2 componentClass(uid, PClassInfo::kManyInstances, kVstAudioEffectClass, Name, 0, Category, Vendor, "1.0.0", kVstVersionString);
		gPluginFactory->registerClass(&componentClass, VST3HAL::createInstance);

		static PClassInfo2 controllerClass(kControllerUID, PClassInfo::kManyInstances, kVstComponentControllerClass, "MyDistortionController", 0, "", Vendor, "1.0.0", kVstVersionString);
		gPluginFactory->registerClass(&controllerClass, VST3HALController::createInstance);
	}
	else
		gPluginFactory->addRef();

	return gPluginFactory;
}

#endif
//
//
//
//static const Steinberg::FUID kVST3AUPlugInProcessorUID(0x301DF339, 0xAFA3533F, 0xB5053B1B, 0x41367137);
//using namespace Steinberg;
//__declspec (dllexport) IPluginFactory* __stdcall GetPluginFactory()
//{
//	if (!gPluginFactory)
//	{
//		static PFactoryInfo factoryInfo("MyCompany", "https://www.mywebsite.com", "mailto:info@mywebsite.com", Vst::kDefaultFactoryFlags);
//		gPluginFactory = new CPluginFactory(factoryInfo);
//
//		{
//			TUID lcid = {
//				(::Steinberg::int8)(((::Steinberg::uint32)(kVST3AUPlugInProcessorUID.getLong1()) & 0x000000FF)),
//				(::Steinberg::int8)(((::Steinberg::uint32)(kVST3AUPlugInProcessorUID.getLong1()) & 0x0000FF00) >> 8),
//				(::Steinberg::int8)(((::Steinberg::uint32)(kVST3AUPlugInProcessorUID.getLong1()) & 0x00FF0000) >> 16),
//				(::Steinberg::int8)(((::Steinberg::uint32)(kVST3AUPlugInProcessorUID.getLong1()) & 0xFF000000) >> 24),
//				(::Steinberg::int8)(((::Steinberg::uint32)(kVST3AUPlugInProcessorUID.getLong2()) & 0x00FF0000) >> 16),
//				(::Steinberg::int8)(((::Steinberg::uint32)(kVST3AUPlugInProcessorUID.getLong2()) & 0xFF000000) >> 24),
//				(::Steinberg::int8)(((::Steinberg::uint32)(kVST3AUPlugInProcessorUID.getLong2()) & 0x000000FF)),
//				(::Steinberg::int8)(((::Steinberg::uint32)(kVST3AUPlugInProcessorUID.getLong2()) & 0x0000FF00) >> 8),
//				(::Steinberg::int8)(((::Steinberg::uint32)(kVST3AUPlugInProcessorUID.getLong3()) & 0xFF000000) >> 24),
//				(::Steinberg::int8)(((::Steinberg::uint32)(kVST3AUPlugInProcessorUID.getLong3()) & 0x00FF0000) >> 16),
//				(::Steinberg::int8)(((::Steinberg::uint32)(kVST3AUPlugInProcessorUID.getLong3()) & 0x0000FF00) >> 8),
//				(::Steinberg::int8)(((::Steinberg::uint32)(kVST3AUPlugInProcessorUID.getLong3()) & 0x000000FF)),
//				(::Steinberg::int8)(((::Steinberg::uint32)(kVST3AUPlugInProcessorUID.getLong4()) & 0xFF000000) >> 24),
//				(::Steinberg::int8)(((::Steinberg::uint32)(kVST3AUPlugInProcessorUID.getLong4()) & 0x00FF0000) >> 16),
//				(::Steinberg::int8)(((::Steinberg::uint32)(kVST3AUPlugInProcessorUID.getLong4()) & 0x0000FF00) >> 8),
//				(::Steinberg::int8)(((::Steinberg::uint32)(kVST3AUPlugInProcessorUID.getLong4()) & 0x000000FF)) };
//			static PClassInfo componentClass(lcid, PClassInfo::kManyInstances, "Audio Module Class", "MyDistortionPlugin");
//			gPluginFactory->registerClass(&componentClass, VST3HAL::createInstance);
//		};
//	}
//	else
//		gPluginFactory->addRef();
//
//	return gPluginFactory;
//}