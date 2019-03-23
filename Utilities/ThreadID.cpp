#include "pch.h"
#include "ThreadID.h"

thread_local ThreadID ThreadID::ourThreadID;
thread_local String ThreadID::ourThreadName;
std::array<ThreadID*, MaxThreadID> ThreadID::ourThreadIDs;
std::mutex ThreadID::ourMutex;

