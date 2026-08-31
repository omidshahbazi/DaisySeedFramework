#ifdef ON_HARDWARE

#include "DaisySeedFramework/USB/DaisyUSBAMCInterface.h"
#include "DaisySeedFramework/USB/DaisyUSB.h"

DaisyUSBAMCInterface::DaisyUSBAMCInterface(DaisyUSB* USB, const Configs& Configs)
	: DaisyUSBInterfaceCommon(USB, Configs)
{}

bool DaisyUSBAMCInterface::OnSetupStage(const USBDeviceSetupPacket* Setup)
{
	return false;
}

void DaisyUSBAMCInterface::OnSetupCompleted(void)
{}

void DaisyUSBAMCInterface::OnDataInStage(void)
{}

void DaisyUSBAMCInterface::OnDataOutStage(void)
{}

void DaisyUSBAMCInterface::BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, uint16& Offset, uint8 InterfaceIndex, const USBClassNode& Class)
{}

uint8 DaisyUSBAMCInterface::CalculateRequiredInterfaceCount(const AMCClassConfig& Class)
{
	return 1 + (Class.OutputChannelCount > 0 ? 1 : 0) + (Class.InputChannelCount > 0 ? 1 : 0);
}

uint16 DaisyUSBAMCInterface::CalculateIsoPacketSize(uint8 ChannelCount, const AMCClassConfig& Class)
{
	if (ChannelCount == 0)
		return 0;

	uint32 bytesPerFrame = ChannelCount * (Class.DefaultBitDepth / 8);
	uint32 bytesPerPacket = bytesPerFrame * (Class.DefaultSampleRate / 1000);

	ASSERT(bytesPerPacket <= 1023, "Packet size exceeds Full-Speed isochronous limit");

	return (uint16)bytesPerPacket;
}

#endif