#pragma once
#ifndef BARRIER_H
#define BARRIER_H

#include "Common.h"
#include <DigitalSignalProcessing/DataTypes.h>

class IHAL;

class Barrier
{
public:
	Barrier(IHAL* HAL)
		: m_HAL(HAL),
		m_Counter(0)
	{}

	void Signal(void)
	{
		m_Counter = m_Counter + 1;

		MEMORY_BARRIER();
	}

	bool Wait(float Timeout = 0) const;

private:
	IHAL* m_HAL;
	volatile uint32_t m_Counter;
};

#endif