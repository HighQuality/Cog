#include "pch.h"
#include "Stopwatch.h"

static u64 GetFrequency()
{
	LARGE_INTEGER frequency;
	if (!QueryPerformanceFrequency(&frequency))
		FATAL(L"QueryPerformanceFrequency failed");
	return frequency.QuadPart;
}

static u64 GetCurrentTimeStamp()
{
	LARGE_INTEGER time;
	if (!QueryPerformanceCounter(&time))
		FATAL(L"QueryPerformanceCounter failed");
	return time.QuadPart;
}

f32 Stopwatch::ourFrequency = static_cast<f32>(GetFrequency());

Stopwatch::Stopwatch()
{
	Restart();
}

void Stopwatch::Restart()
{
	myStartTime = GetCurrentTimeStamp();
}

u64 Stopwatch::GetElapsedTicks() const
{
	return GetCurrentTimeStamp() - myStartTime;
}
