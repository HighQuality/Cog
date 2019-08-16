#include "pch.h"
#include "EntityInitializer.h"
#include "Entity.h"

void EntityInitializer::InitializeEntity()
{
	// Double initialization
	CHECK(!myWasInitialized);
	myWasInitialized = true;

	myEntity->ResolveDependencies(*this);
	myEntity->Initialize();
}
