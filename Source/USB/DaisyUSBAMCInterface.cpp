#ifdef ON_HARDWARE

#include "DaisySeedFramework/USB/DaisyUSBAMCInterface.h"
#include "DaisySeedFramework/USB/DaisyUSB.h"

DaisyUSBAMCInterface::DaisyUSBAMCInterface(DaisyUSB* USB, uint16 InterfaceIndexMask, uint8 EndpointCommand, uint8 EndpointIn, uint8 EndpointOut)
	: DaisyUSBInterfaceCommon(USB, InterfaceIndexMask, EndpointCommand, EndpointIn, EndpointOut)
{}

bool DaisyUSBAMCInterface::OnSetupStage(const USBDeviceSetupPacket* Setup)
{}

void DaisyUSBAMCInterface::OnDataOutStage(void)
{}

void DaisyUSBAMCInterface::BuildConfigurationDescriptor(EP0Buffer& EP0Buffer, uint16& Offset, uint8& InterfaceIndex, uint8 Interval, const CDCClassConfig& Config)
{}

#endif