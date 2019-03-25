#include "pch.h"
#include "DebugAsserts.h"
#include <Windows.h>

void EnsureLog(const char* aCondition)
{
	std::cout << "ensure(" << aCondition << ") failed" << std::endl;
}

void ErrorLog(StringView aMessage)
{
	OutputDebugStringW(aMessage.GetData());
	std::wcout << aMessage << std::flush;
}
