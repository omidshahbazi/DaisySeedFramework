#pragma once
#ifndef VERSION_H
#define VERSION_H

#include <DigitalSignalProcessing/DataTypes.h>

struct Version
{
public:
	constexpr Version(uint8_t Major = 0, uint8_t Minor = 0, uint8_t Build = 0, uint8_t Revision = 0)
		: m_Major(Major),
		m_Minor(Minor),
		m_Build(Build),
		m_Revision(Revision)
	{}

	bool operator ==(Version Other);
	bool operator <(Version Other);

	cstr ToString(bool NumberOnly = false);

public:
	union
	{
		struct
		{
			uint8_t m_Revision;
			uint8_t m_Build;
			uint8_t m_Minor;
			uint8_t m_Major;
		};

		uint32_t m_Number;
	};
};

#endif