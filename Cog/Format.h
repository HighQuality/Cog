#pragma once
#include "Append.h"

inline void UnpackAppendToString(String& aString, i32)
{
	aString.Append(L"???");
}

template <typename TArg, typename ...TArgs>
void UnpackAppendToString(String& aString, const i32 aArgNum, const TArg& aArg, const TArgs& ...aArgs)
{
	if (aArgNum == 0)
	{
		Append(aString, aArg);
		return;
	}

	UnpackAppendToString(aString, aArgNum - 1, aArgs...);
}

template <typename ...TArgs>
String Format(const StringView& aFormat, const TArgs& ...aArgs)
{
	if (sizeof...(aArgs) == 0)
		return String(aFormat);

	// TODO: Reserve output buffer
	String output;

	i32 aArgNum = 0;

	for (i32 i = 0; i < aFormat.GetLength(); ++i)
	{
		if (aFormat[i] == L'%')
		{
			if (aFormat.TryGet(i + 1, L' ') == L'%')
			{
				++i;
				output.Add(L'%');
				continue;
			}

			UnpackAppendToString(output, aArgNum++, aArgs...);
		}
		else
		{
			// TODO: Don't add individually, save begin and append a slice instead
			output.Add(aFormat[i]);
		}
	}

	while (aArgNum < sizeof...(aArgs))
	{
		UnpackAppendToString(output, aArgNum++, aArgs...);
	}

	return output;
}
