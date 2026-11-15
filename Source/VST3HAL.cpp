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
#include <DigitalSignalProcessing/Debug.h>

using namespace Steinberg;
using namespace Steinberg::Vst;

#pragma comment(lib, "sdk.lib")
#pragma comment(lib, "sdk_common.lib")
#pragma comment(lib, "base.lib")
#pragma comment(lib, "pluginterfaces.lib")

#undef ASSERT
#define ASSERT DSP_ASSERT

static IPluginEvents* g_VST3HAL = nullptr;

class Line1View : public CPluginView
{
public:
	Line1View(void)
		: CPluginView(&m_Rect)
	{
		Point size = g_VST3HAL->GetViewSize();

		m_Rect = ViewRect(0, 0, size.X, size.Y);
	}

	void attachedToParent(void) override
	{
		g_VST3HAL->OnSetViewEnabled(true, systemWindow);

		if (plugFrame != nullptr)
			plugFrame->resizeView(this, &m_Rect);
	}

	void removedFromParent(void) override
	{
		g_VST3HAL->OnSetViewEnabled(false);
	}

	tresult PLUGIN_API isPlatformTypeSupported(FIDString Type) override
	{
		return strcmp(Type, kPlatformTypeHWND) == 0 ? kResultTrue : kResultFalse;
	}

	tresult PLUGIN_API setFrame(IPlugFrame* f) override
	{
		CPluginView::setFrame(f);

		return kResultOk;
	}

	tresult PLUGIN_API onSize(ViewRect* Rect) override
	{
		rect = m_Rect;

		return kResultOk;
	}

	tresult PLUGIN_API canResize(void) override
	{
		return kResultFalse;
	}

	tresult PLUGIN_API checkSizeConstraint(ViewRect* Rect) override
	{
		if (Rect != nullptr)
			*Rect = m_Rect;

		return kResultOk;
	}

private:
	ViewRect m_Rect;
};

class VST3HALController : public EditController
{
public:
	IPlugView* PLUGIN_API createView(FIDString Name) override
	{
		if (Name && strcmp(Name, ViewType::kEditor) == 0)
			return new Line1View();

		return nullptr;
	}
};

class VST3HALPlugin : public AudioEffect
{
public:
	VST3HALPlugin(const FUID& ControllerUID)
	{
		setControllerClass(ControllerUID);
	}

	tresult PLUGIN_API initialize(FUnknown* Context) override
	{
		tresult result = AudioEffect::initialize(Context);
		if (result != kResultOk)
			return result;

		addAudioInput(STR16("In"), SpeakerArr::kStereo);
		addAudioOutput(STR16("Out"), SpeakerArr::kStereo);

		static bool isInitialized = false;
		if (!isInitialized)
		{
			g_VST3HAL->OnInitialize();

			isInitialized = true;
		}

		m_StopUpdateThread = false;
		m_UpdateThread = std::thread([this]
			{
				while (!m_StopUpdateThread)
				{
					g_VST3HAL->OnUpdate();

					std::this_thread::sleep_for(std::chrono::milliseconds(1));
				}
			});

		return kResultOk;
	}

	tresult PLUGIN_API terminate(void) override
	{
		m_StopUpdateThread = true;
		if (m_UpdateThread.joinable())
			m_UpdateThread.join();

		return AudioEffect::terminate();
	}

	tresult PLUGIN_API process(ProcessData& Data) override
	{
		if (Data.numInputs == 0 || Data.numOutputs == 0 || Data.numSamples == 0)
			return kResultOk;

		g_VST3HAL->OnProcessAudio(Data.inputs[0].channelBuffers32, Data.outputs[0].channelBuffers32, Data.numSamples);

		return kResultOk;
	}

	tresult PLUGIN_API setBusArrangements(SpeakerArrangement* Inputs, int32 InputBusCount, SpeakerArrangement* Outputs, int32 OutputBusCount) override
	{
		if (InputBusCount == 1 && OutputBusCount == 1 && Inputs[0] == SpeakerArr::kStereo && Outputs[0] == SpeakerArr::kStereo)
			return AudioEffect::setBusArrangements(Inputs, InputBusCount, Outputs, OutputBusCount);

		return kResultFalse;
	}

private:
	std::thread m_UpdateThread;
	std::atomic_bool m_StopUpdateThread;
};

VST3HAL::VST3HAL(const CreationInfo& Info)
	: WindowsHAL(Info),
	m_FrameLength(0)
{
	g_VST3HAL = this;
}

void VST3HAL::Setup(const SetupInfo& Info)
{
	ASSERT(Info.FrameLength != 0, "Invalid FrameLength %i", Info.FrameLength);
	ASSERT(Info.SampleRate != 0, "Invalid SampleRate %i", Info.SampleRate);
	ASSERT(Info.Vendor != nullptr, "Invalid Vendor");
	ASSERT(Info.URL != nullptr, "Invalid URL");
	ASSERT(Info.Email != nullptr, "Invalid Email");
	ASSERT(Info.ID != 0, "Invalid ID %u", Info.ID);
	ASSERT(Info.Name != nullptr, "Invalid Name");

	m_OnInitialize = Info.OnInitialize;
	m_OnTerminate = Info.OnTerminate;
	m_OnUpdate = Info.OnUpdate;
	m_FrameLength = Info.FrameLength;
	m_OnGetViewSize = Info.OnGetViewSize;
	m_OnSetViewEnabled = Info.OnSetViewEnabled;

	static const FUID componentUID(Info.ID, Info.ID >> 8, Info.ID >> 16, Info.ID >> 24);
	static const FUID controlUID(Info.ID >> 24, Info.ID >> 16, Info.ID >> 8, Info.ID);

	if (gPluginFactory == nullptr)
	{
		PFactoryInfo factoryInfo(Info.Vendor, Info.URL, Info.Email, kDefaultFactoryFlags);
		gPluginFactory = new CPluginFactory(factoryInfo);

		static PClassInfo2 componentClass(componentUID, 1, kVstAudioEffectClass, Info.Name, 0, "Fx", Info.Vendor, Info.Version.ToString(), kVstVersionString);
		gPluginFactory->registerClass(&componentClass, [](void* ptr)->FUnknown* { return (IAudioProcessor*)new VST3HALPlugin(controlUID); });

		static PClassInfo2 controllerClass(controlUID, 1, kVstComponentControllerClass, Info.Name, 0, "", Info.Vendor, Info.Version.ToString(), kVstVersionString);
		gPluginFactory->registerClass(&controllerClass, [](void* ptr)->FUnknown* { return (IEditController*)new VST3HALController(); });
	}
}

void VST3HAL::OnProcessAudio(const float* const* In, float** Out, uint32_t Size)
{
	for (uint32_t pos = 0; pos < Size; )
	{
		uint8_t chunk = Math::Min<uint32_t>(Size - pos, m_FrameLength);

		const float* ins[2] = { In[0] + pos, In[1] + pos };
		float* outs[2] = { Out[0] + pos, Out[1] + pos };

		m_AudioCallback(ins, outs, chunk);

		pos += chunk;
	}
}

bool VST3HAL::TearDown(void)
{
	return true;
}

bool VST3HAL::InitializeModule(void)
{
	return true;
}

bool VST3HAL::TerminateModule(void)
{
	g_VST3HAL->OnTerminate();

	return true;
}

void* VST3HAL::GetFactory(void)
{
	return gPluginFactory;
}

#endif