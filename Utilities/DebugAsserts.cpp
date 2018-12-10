#include "pch.h"
#include "DebugAsserts.h"
#include <iostream>

void EnsureLog(const char* aCondition)
{
	std::cout << "ensure(" << aCondition << ") failed" << std::endl;
}
