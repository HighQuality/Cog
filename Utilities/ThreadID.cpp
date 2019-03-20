#include "pch.h"
#include "ThreadID.h"

thread_local ThreadID ThreadID::ourThreadID;
std::array<ThreadID*, MaxThreadID> ThreadID::ourThreadIDs;
std::mutex ThreadID::ourMutex;

