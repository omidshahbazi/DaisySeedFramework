#include "DaisySeedFramework/Barrier.h"
#include <DigitalSignalProcessing/IHAL.h>

bool Barrier::Wait(float Timeout) const
{
	const uint32_t counter = m_Counter;
	const float Deadline = m_HAL->GetTimeSinceStartup() + Timeout;

	while (m_Counter == counter)
	{
		if (Timeout != 0 && m_HAL->GetTimeSinceStartup() >= Deadline)
			return false;
	}

	MEMORY_BARRIER();

	return true;
}