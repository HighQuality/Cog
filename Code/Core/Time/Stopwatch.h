#pragma once
#include <Function/FunctionView.h>
#include <Time/TimeSpan.h>

class Stopwatch
{
public:
	Stopwatch();
	~Stopwatch() = default;

	void Restart();

	// Gets the elapsed number of ticks using the highest available frequency, note that these are not the same kind of ticks used in TimeSpan
	FORCEINLINE u64 GetElapsedNativeTicks() const
	{
		return GetPerformanceCounter() - myStartTime;
	}

	FORCEINLINE static u64 GetNativeTicksPerSecond()
	{
		return ourNativeTicksPerSecond;
	}

	FORCEINLINE TimeSpan GetElapsedTime() const
	{
		return TimeSpan::FromTicks(static_cast<i64>(static_cast<f64>(GetElapsedNativeTicks()) * ourInvNativeTicksPerTimeSpanTick));
	}

	static TimeSpan Time(FunctionView<void()> aFunction, StringView name, bool printResult = true)
	{
		Stopwatch w;
		aFunction();
		const auto result = w.GetElapsedTime();
		if (printResult)
			Println(L"% took %ms to execute", name, result.GetTotalMilliseconds());
		return result;
	}

	FORCEINLINE static u64 GetPerformanceCounter()
	{
		LARGE_INTEGER time;
		QueryPerformanceCounter(&time);
		return time.QuadPart;
	}

private:
	static u64 ourNativeTicksPerSecond;
	static f64 ourInvNativeTicksPerTimeSpanTick;

	u64 myStartTime;
};
