#include "CorePch.h"
#include "Stopwatch.h"

static u64 GetPerformanceCounterFrequency()
{
	LARGE_INTEGER frequency;
	if (!QueryPerformanceFrequency(&frequency))
		FATAL(L"QueryPerformanceFrequency failed");
	return frequency.QuadPart;
}

u64 Stopwatch::ourNativeTicksPerSecond = GetPerformanceCounterFrequency();
// NOTE: Since Windows 10 version 1809 this always seems to be 1, could potentially be used for a fast-path, google "queryperformancefrequency 10 mhz" for more info
f64 Stopwatch::ourInvNativeTicksPerTimeSpanTick = 1.0 / (ourNativeTicksPerSecond / 10000000.0);

Stopwatch::Stopwatch()
{
	Restart();
}

void Stopwatch::Restart()
{
	myStartTime = GetPerformanceCounter();
}
