#pragma once
#include "TypeID.h"
#include "ThreadID.h"
#include "BaseFactory.h"
#include "CogStack.h"

class ThreadPool;
class Fiber;
class Awaitable;

template <typename T>
BaseFactory& AllocateFactory();

template <typename T>
T& DefaultAllocate();
template <typename T>
void DefaultFree(T& aObject);

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

class Program
{
public:
	Program();
	~Program();

	void Run();

	static Program& Create();

	static Program& Get();

	template <typename T>
	T& AllocateUninitialized()
	{
		return *static_cast<T*>(AllocateRaw(TypeID<void>::Resolve<T>(), &AllocateFactory<T>));
	}

	template <typename T>
	void ReturnUninitialized(T& aObject)
	{
		Return(TypeID<void>::Resolve<T>(), &aObject);
	}

	bool IsInMainThread() const { return myMainThread == ThreadID::Get(); }
	bool IsInManagedThread() const;

	void RegisterUnusedFiber(Fiber* aFiber);

	void QueueWork(void(*aFunction)(void*), void* aArgument);
	void QueueFiber(Fiber* aFiber);

	void QueueBackgroundWork(void(*aFunction)(void*), void* aArgument);

	Fiber* GetUnusedFiber();

private:
	void* AllocateRaw(TypeID<void> aTypeID, BaseFactory&(*aFactoryAllocator)());
	void Return(TypeID<void> aTypeID, void* aObject);
	void WorkerThread(i32 aThreadIndex);

	void FiberMain();

	const ThreadID& myMainThread;
	// TODO: Change key to TypeID<void>
	Map<TypeID<void>::CounterType, BaseFactory*> myAllocators;

	ThreadPool* myBackgroundWorkThreadPool;

	std::mutex myFiberMutex;
	Stack<Fiber*> myUnusedFibers;

	std::mutex myWorkMutex;
	std::condition_variable myWorkNotify;
	Array<std::thread> myWorkers;

	bool myIsStopping = false;

	std::condition_variable myWakeMainNotify;
	std::mutex myWakeMainMutex;

	i32 mySleepingThreads = 0;

	i32 myNumWorkers;

	struct QueuedWork
	{
		void(*function)(void*) = nullptr;
		void* argument = nullptr;
	};

	Array<QueuedWork> myWorkQueue;
	Array<Fiber*> myQueuedFibers;

	bool myIsMainRunning = true;
};

extern Program* gProgram;

FORCEINLINE Program& Program::Get()
{
	return *gProgram;
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
