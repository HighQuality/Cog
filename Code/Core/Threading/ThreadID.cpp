#include "CorePch.h"
#include "ThreadID.h"

std::array<ThreadID*, MaxThreadID> ThreadID::ourThreadIDs;

void ThreadID::SetName(String aThreadName)
{
	aThreadName.CheckEndsWithZero();
	SetThreadDescription(GetCurrentThread(), aThreadName.GetData());
	CoreTLS::SetThreadName(Move(aThreadName));
}

std::mutex ThreadID::ourMutex;
