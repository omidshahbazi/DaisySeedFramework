#include "DaisySeedFramework/Version.h"
#include "DaisySeedFramework/StringUtils.h"
#include <string>

bool Version::operator==(Version Other)
{
	return (m_Number == Other.m_Number);
}

bool Version::operator<(Version Other)
{
	return (m_Number < Other.m_Number);
}

cstr Version::ToString(bool NumberOnly)
{
	static std::string value;

	if (NumberOnly)
		return ::ToString(m_Number);

	value = "";
	value += ::ToString(m_Major);
	value += '.';

	value += ::ToString(m_Minor);
	value += '.';

	value += ::ToString(m_Build);
	value += '.';

	value += ::ToString(m_Revision);

	return value.c_str();
}