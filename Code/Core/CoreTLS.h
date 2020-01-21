#pragma once

class ThreadID;
class String;
class StringView;
struct pcg32;

namespace CoreTLS
{
	NOINLINE void SetThreadID(ThreadID* aNewThreadID);
	NOINLINE const ThreadID* GetThreadID();

	NOINLINE void SetThreadName(String aThreadName);
	NOINLINE StringView GetThreadName();

	NOINLINE pcg32& GetRandomEngine();
}
