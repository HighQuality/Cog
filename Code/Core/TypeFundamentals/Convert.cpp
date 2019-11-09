#include "CorePch.h"
#include "Convert.h"

bool StringToU64(const StringView& aString, u64& result)
{
	for (i32 i = 0; i < aString.GetLength(); ++i)
	{
		if (!isdigit(aString[i]))
			return false;
	}
	// TODO: Get rid of allocation
	result = std::stoull(std::wstring(aString.GetData(), aString.GetLength()));
	return true;
}

bool StringToI64(const StringView& aString, i64& result)
{
	for (i32 i = 0; i < aString.GetLength(); ++i)
	{
		if (!isdigit(aString[i]))
		{
			if (aString[i] == L'-' && i == 0)
				continue;
			return false;
		}
	}
	// TODO: Get rid of allocation
	result = std::stoll(std::wstring(aString.GetData(), aString.GetLength()));
	return true;
}
