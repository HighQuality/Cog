#include "CorePch.h"
#include "DebugAsserts.h"
#include <Windows.h>

void EnsureLog(const StringView aMessage)
{
	OutputDebugStringW(aMessage.GetData());
	std::wcout << aMessage << std::flush;
}

void ErrorLog(const StringView aMessage)
{
	OutputDebugStringW(aMessage.GetData());
	std::wcout << aMessage << std::flush;
}
