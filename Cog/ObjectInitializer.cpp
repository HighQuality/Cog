#include "pch.h"
#include "ObjectInitializer.h"
#include "Object.h"

void ObjectInitializer::InitializeObject()
{
	// Double initialization
	CHECK(!wasInitialized);
	wasInitialized = true;

	myObject->ResolveDependencies(*this);
	myObject->Initialize();
}
