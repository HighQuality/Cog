#include "pch.h"
#include "TestComponent.h"

void TestComponent::Tick(Time aDeltaTime)
{
	Synchronize(*this, &TestComponent::Callback);
}

void TestComponent::Callback()
{
	Println(L"Callback!");
}
