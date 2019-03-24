#include "pch.h"
#include "Fiber.h"
#include "FiberLocalStorage.h"
#include "ThreadID.h"
#include "CogStack.h"

thread_local void* Fiber::ourFiberHandle = nullptr;

static std::mutex gFiberIndexMutex;
static std::unordered_map<void*, String> gFiberIndex;

thread_local Stack<void*> previousFiberStack;

thread_local StringView gCurrentFiber;

static void SwitchFiber(FiberHandle aFiber)
{
	// void* currentFiber = GetCurrentFiber();
	// String threadName = String(ThreadID::GetName());
	// String currentFiberName = gFiberIndex[currentFiber];
	// String targetFiber = gFiberIndex[aFiber.GetHandle()];

	// Println(L"Thread % (fiber %) switching to fiber %", threadName, currentFiberName, targetFiber);

	// previousFiberStack.Push(GetCurrentFiber());

	// Println(L"Switching to fiber %", gFiberIndex[aFiber.GetHandle()]);

	SwitchToFiber(aFiber.GetHandle());

	// void* previousFiber = previousFiberStack.Pop();

	// CHECK(previousFiber == GetCurrentFiber());

	//Println(L"Thread % returned from fiber % to fiber %", threadName, gFiberIndex[previousFiber], gFiberIndex[currentFiber]);

	// {
	// 	StringView currentFiber = gFiberIndex[Fiber::GetCurrentlyExecutingFiber()->GetFiberHandle()];
	// 	StringView callingFiber = gFiberIndex[aFiber.GetHandle()];
	// 	Println(L"Thread % (fiber %) switching to fiber %", ThreadID::GetName(), currentFiber, callingFiber);
	// }
}

void Fiber::ExecuteFiberLoop(void* aPtr)
{
	Fiber& fiber = *static_cast<Fiber*>(aPtr);

	for (;;)
	{
		// Fiber was executed without having any work provided
		CHECK(fiber.myCurrentWork);
		fiber.myCurrentWork(fiber.myArgument);
		fiber.myCurrentWork = nullptr;
		fiber.myArgument = nullptr;

		SwitchFiber(fiber.myCallingFiber);
	}
}

std::atomic<i32> fiberNum = 1;

Fiber::Fiber()
{
	// 16 KB stack size
	myFiberHandle = FiberHandle(CreateFiber(32 * 1024, &Fiber::ExecuteFiberLoop, this));

	CHECK(myFiberHandle.GetHandle());

	{
		// TODO: Remove/disable for performance, only used for debugging
		std::unique_lock<std::mutex> lck(gFiberIndexMutex);
		gFiberIndex[myFiberHandle.GetHandle()] = Format(L"Fiber ", fiberNum.fetch_add(1));
	}
}

void Fiber::SetCurrentWork(const StringView& aWork)
{
	GetCurrentlyExecutingFiber()->SetWork(aWork);
}

void Fiber::SetWork(const StringView& aWork)
{
	std::unique_lock<std::mutex> lck(gFiberIndexMutex);
	gFiberIndex.find(myFiberHandle.GetHandle())->second = aWork;
}

Fiber::~Fiber()
{
	if (myFiberHandle.GetHandle())
	{
		DeleteFiber(myFiberHandle.GetHandle());
		myFiberHandle = FiberHandle();
	}
}

bool Fiber::Execute(void (*aWork)(void*), void* aArgument)
{
	StartWork(aWork, aArgument);
	return Continue();
}

void Fiber::StartWork(void (*aWork)(void*), void* aArgument)
{
	CHECK(!HasWork());

	// myYieldedData = nullptr;
	myCurrentWork = aWork;
	myArgument = aArgument;
}

bool Fiber::Continue()
{
	CHECK(HasWork());

	// This fiber is already being worked on by someone else, some other thread most likely tried to continue simultaneously
	CHECK(!myCallingFiber);

	// You must call Fiber::ConvertCurrentThreadToFiber before working on fibers
	CHECK(ourFiberHandle);

	myYieldedData = nullptr;
	myCallingFiber = FiberHandle(GetCurrentFiber());

	CHECK(myCallingFiber);

	// Causes unpredictable problems according to msdn: SwitchToFiber(GetCurrentFiber()), probably won't want this happening
	// If you hit this, fibers can't continue on themselves
	CHECK(myCallingFiber != myFiberHandle);

	SwitchFiber(myFiberHandle);

	myCallingFiber.Reset();

	// Return true when we still have work to do
	return myCurrentWork != nullptr;
}

void Fiber::YieldExecution(void* yieldData)
{
	Fiber* fiberPtr = GetCurrentlyExecutingFiber();

	// Was not called from a fiber
	CHECK(fiberPtr);

	Fiber& fiber = *fiberPtr;

	// Fiber yielded without having any work (executed without being supposed to, spooky)
	CHECK(fiber.myCallingFiber);

	FiberHandle callingFiber = fiber.myCallingFiber;

	// Fiber is executing recursively
	CHECK(fiber.myYieldedData == nullptr);

	fiber.myYieldedData = yieldData;

	SwitchFiber(callingFiber);

	fiber.myYieldedData = nullptr;
}

void Fiber::ConvertCurrentThreadToFiber(const StringView & aDescription)
{
	if (ourFiberHandle == nullptr)
	{
		ourFiberHandle = ConvertThreadToFiber(nullptr);

		{
			// TODO: Remove/disable for performance, only used for debugging
			std::unique_lock<std::mutex> lck(gFiberIndexMutex);
			gFiberIndex[ourFiberHandle] = aDescription;
		}
	}
}

Fiber* Fiber::GetCurrentlyExecutingFiber()
{
	return static_cast<Fiber*>(GetFiberData());
}
