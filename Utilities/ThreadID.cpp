#include "pch.h"
#include "ThreadID.h"

thread_local ThreadID ThreadID::ourThreadID;
std::mutex ThreadID::ourMutex;
std::array<ThreadID*, MaxThreadID> ThreadID::ourThreadIDs;
