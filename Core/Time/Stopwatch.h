#pragma once
#include "FunctionView.h"
#include "CogTime.h"

class Stopwatch
{
public:
	Stopwatch();
	~Stopwatch() = default;

	void Restart();

	u64 GetElapsedTicks() const;

	FORCEINLINE static f32 GetTickFrequency()
	{
		return ourFrequency;
	}

	FORCEINLINE Time GetElapsedTime() const
	{
		return Time::Seconds(static_cast<f32>(GetElapsedTicks()) / ourFrequency);
	}

	static Time Time(FunctionView<void()> aFunction, StringView name, bool printResult = true)
	{
		Stopwatch w;
		aFunction();
		const auto result = w.GetElapsedTime();
		if (printResult)
			Println(L"% took %ms to execute", name, result.Milliseconds());
		return result;
	}

	FORCEINLINE static u64 GetCurrentTimeStamp()
	{
		LARGE_INTEGER time;
		if (!QueryPerformanceCounter(&time))
			FATAL(L"QueryPerformanceCounter failed");
		return time.QuadPart;
	}


private:
	static f32 ourFrequency;

	u64 myStartTime;
};
