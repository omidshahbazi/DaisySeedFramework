#include "DaisySeedFramework/StringUtils.h"
#include <DigitalSignalProcessing/Notes.h>
#include <DigitalSignalProcessing/Math.h>
#include <DigitalSignalProcessing/Memory.h>
#include <stdio.h>

cstr IntToString(int32_t Value)
{
	static char buffer[8];

	snprintf(buffer, sizeof(buffer), "%i", Value);

	return buffer;
}

cstr FloatToString(float Value, uint8_t Decimals)
{
	static char buffer[16];

	uint8_t idx = 0;

	if (Decimals < 0)
		Decimals = 0;
	if (Decimals > 4)
		Decimals = 4;

	if (Value < 0)
	{
		buffer[idx++] = '-';
		Value = -Value;
	}

	static const uint32_t pow10[] = { 1, 10, 100, 1000, 10000 };
	uint32_t mult = pow10[Decimals];

	uint32_t scaled = (uint32_t)roundf(Value * mult);
	uint32_t int_part = scaled / mult;
	uint32_t frac_part = scaled % mult;

	uint32_t len = snprintf(buffer + idx, sizeof(buffer) - idx, "%d", int_part);
	if (len < 0)
		return buffer;

	idx += len;

	if (Decimals > 0 && frac_part > 0)
	{
		uint8_t active_decimals = Decimals;
		while (active_decimals > 0 && frac_part % 10 == 0)
		{
			frac_part /= 10;
			active_decimals--;
		}

		if (active_decimals > 0)
		{
			buffer[idx++] = '.';

			char temp[5];
			for (uint32_t i = 0; i < active_decimals; i++)
			{
				temp[active_decimals - 1 - i] = '0' + (frac_part % 10);
				frac_part /= 10;
			}

			for (uint32_t i = 0; i < active_decimals; i++)
				buffer[idx++] = temp[i];
		}
	}

	buffer[idx] = '\0';

	return buffer;
}

uint8_t GetNoteIndex(uint8_t MIDINumber)
{
	return Math::Moderate(MIDINumber, TotalNoteCount);
}

cstr GetEnglishNoteName(uint8_t MIDINumber)
{
	return EnglishNoteNames[GetNoteIndex(MIDINumber)];
}

cstr GetFrenchNoteName(uint8_t MIDINumber)
{
	return FrenchNoteNames[Math::Moderate(MIDINumber, TotalNoteCount)];
}