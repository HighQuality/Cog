#pragma once
#include "TypeID.h"
#include "ThreadID.h"
#include "EventList.h"

class BaseProgramAllocator
{
public:
	virtual void* AllocateRawObject() = 0;
	virtual void ReturnRawObject(void* aObject) = 0;

protected:
	virtual ~BaseProgramAllocator();
};

template <typename T>
BaseProgramAllocator& AllocateFactory();

template <typename T>
T& DefaultAllocate();
template <typename T>
void DefaultFree(T& aObject);

void YieldExecution();

template <typename T, typename ...TArgs>
T& Allocate(TArgs ...aArgs)
{
	T& object = DefaultAllocate<T>();
	new (static_cast<void*>(&object)) T(std::forward<TArgs>(aArgs)...);
	return object;
}

template <typename T>
void Free(T& aObject)
{
	aObject.~T();
	DefaultFree<T>(aObject);
}

// TODO: Figure out how to get a rate from this without it being virtual
class ITickRate
{
protected:
	ITickRate() = default;
	// Left non-virtual on purpose
	~ITickRate() = default;
};

class Program
{
public:
	Program(const ThreadID& aThreadID);
	~Program();

	void Run();

	static Program& Get();

	template <typename T>
	T& AllocateUninitialized()
	{
		return *static_cast<T*>(Allocate(TypeID<void>::Resolve<T>(), &AllocateFactory<T>));
	}

	template <typename T>
	void ReturnUninitialized(T& aObject)
	{
		Return(TypeID<void>::Resolve<T>(), &aObject);
	}
	
	bool IsInMainThread() const { return myMainThread == ThreadID::Get(); }
	bool IsExecutionSynchronized() const { return myExecutionIsSynchronized; }

	void YieldExecution(const ThreadID& aThreadId = ThreadID::Get());
	
private:
	void* Allocate(TypeID<void> aTypeID, BaseProgramAllocator&(*aFactoryAllocator)());
	void Return(TypeID<void> aTypeID, void* aObject);
	void WorkerThread(i32 aWorkerId);
	void WakeMain();

	u8 myWorkerYieldDepth[MaxThreadID];
	ThreadIDInteger myWorkerThreadIds[MaxThreadID];

	const ThreadID& myMainThread;
	// TODO: Change key to TypeID<void>
	Map<TypeID<void>::CounterType, BaseProgramAllocator*> myAllocators;

	std::mutex myWorkMutex;
	std::condition_variable myWorkNotify[MaxThreadID];
	Array<std::thread> myWorkers;
	Array<void(*)(void*)> myCurrentWorkQueue;
	bool myIsStopping = false;

	std::condition_variable myWakeMainNotify;
	std::mutex myWakeMainMutex;
	bool myWakeMainFlag = false;

	i32 mySleepingThreads;

	i32 myNumWorkers;


	EventList<void(*)(void*)> myQueuedWork;

	bool myExecutionIsSynchronized = false;
};

extern Program gProgram;

FORCEINLINE Program& Program::Get()
{
	return gProgram;
}

template <typename T>
T& DefaultAllocate()
{
	// TODO: Use tag on T in order to figure out whether or not to use synchronized allocation (supports iterations)
	return Program::Get().AllocateUninitialized<T>();
}

template <typename T>
void DefaultFree(T& aObject)
{
	// TODO: Use tag on T in order to figure out whether or not to use synchronized allocation (supports iterations)
	Program::Get().ReturnUninitialized<T>(aObject);
}
