#include "pch.h"
#include "Fiber.h"
#include "FiberLocalStorage.h"

thread_local void* Fiber::ourFiberHandle = nullptr;

static std::mutex gFiberIndexMutex;
static std::unordered_map<void*, String> gFiberIndex;

void Fiber::ExecuteFiberLoop(void* aPtr)
{
	Fiber& fiber = *static_cast<Fiber*>(aPtr);

	for (;;)
	{
		// Fiber was executed without having any work provided
		CHECK (fiber.myCurrentWork);
		fiber.myCurrentWork(fiber.myArgument);
		fiber.myCurrentWork = nullptr;
		fiber.myArgument = nullptr;
		
		SwitchToFiber(fiber.myCallingFiber);
	}
}

Fiber::Fiber()
{
	// 16 KB stack size
	myFiberHandle = CreateFiber(16 * 1024, &Fiber::ExecuteFiberLoop, this);
	
	CHECK(myFiberHandle);
	
	{
		// TODO: Remove/disable for performance, only used for debugging
		std::unique_lock<std::mutex> lck(gFiberIndexMutex);
		gFiberIndex[myFiberHandle] = L"Fiber";
	}
}

void Fiber::SetCurrentWork(const StringView& aWork)
{
	std::unique_lock<std::mutex> lck(gFiberIndexMutex);
	gFiberIndex.find(ourFiberHandle)->second = L"Fiber";
}

Fiber::~Fiber()
{
	if (myFiberHandle)
		DeleteFiber(myFiberHandle);
}

bool Fiber::Execute(void (*aWork)(void*), void* aArgument)
{
	StartWork(aWork, aArgument);
	return Continue();
}

void Fiber::StartWork(void (*aWork)(void*), void* aArgument)
{
	CHECK(!HasWork());

	myCurrentWork = aWork;
	myArgument = aArgument;
}

bool Fiber::Continue()
{
	CHECK(HasWork());
	
	// This fiber is already being worked on by someone else, some other thread most likely tried to continue simultaneously
	CHECK(myCallingFiber == nullptr);

	// You must call Fiber::ConvertCurrentThreadToFiber before working on fibers
	CHECK(ourFiberHandle);

	myYieldedData = nullptr;
	myCallingFiber = GetCurrentFiber();

	CHECK(myCallingFiber);

	// Causes unpredictable problems according to msdn: SwitchToFiber(GetCurrentFiber()), probably won't want this happening
	// If you hit this, fibers can't continue on themselves
	CHECK(myCallingFiber != myFiberHandle);
	
	SwitchToFiber(myFiberHandle);
	
	myCallingFiber = nullptr;

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

	void* callingFiber = fiber.myCallingFiber;

	fiber.myYieldedData = yieldData;

	SwitchToFiber(callingFiber);
	
	fiber.myYieldedData = nullptr;
}

void Fiber::ConvertCurrentThreadToFiber(const StringView& aDescription)
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
