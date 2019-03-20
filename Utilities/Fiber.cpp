#include "pch.h"
#include "Fiber.h"
#include "FiberLocalStorage.h"

thread_local void* Fiber::ourFiberHandle = nullptr;

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
	myFiberHandle = CreateFiber(0, &Fiber::ExecuteFiberLoop, this);

	CHECK(myFiberHandle);
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
	
	myYieldedData = nullptr;
	myCallingFiber = ourFiberHandle;
	
	if (myCallingFiber == nullptr)
	{
		ourFiberHandle = ConvertThreadToFiber(nullptr);

		if (ourFiberHandle == nullptr)
		{
			Println(L"ConvertThreadToFiber failed, assuming thread is already fiber...");
			ourFiberHandle = GetCurrentFiber();
		}

		myCallingFiber = ourFiberHandle;
		CHECK(myCallingFiber);
	}
	else
	{
		// Causes unpredictable problems according to msdn: SwitchToFiber(GetCurrentFiber()), probably won't want this happening
		// If you hit this, fibers can't continue on themselves
		CHECK(GetCurrentFiber() != myFiberHandle);
	}

	SwitchToFiber(myFiberHandle);

	myCallingFiber = nullptr;
	
	// Return true when we still have work to do
	return myCurrentWork != nullptr;
}

void Fiber::YieldExecution(void* yieldData)
{
	auto fiberPtr = static_cast<Fiber*>(GetFiberData());

	// Was not called from a fiber
	CHECK(fiberPtr);

	Fiber& fiber = *fiberPtr;

	// Only the fiber itself may yield execution
	CHECK(GetCurrentFiber() == fiber.myFiberHandle);

	// Fiber yielded without having any work (executed without being supposed to, spooky)
	CHECK(fiber.myCallingFiber);

	void* callingFiber = fiber.myCallingFiber;

	fiber.myYieldedData = yieldData;

	SwitchToFiber(callingFiber);
}

void Fiber::ConvertCurrentThreadToFiber()
{
	if (ourFiberHandle == nullptr)
	{
		ourFiberHandle = ConvertThreadToFiber(nullptr);
	}
}
