#include "pch.h"
#include "EntityInitializer.h"
#include "Entity.h"

void EntityInitializer::InitializeEntity()
{
	// Double initialization
	CHECK(!wasInitialized);
	wasInitialized = true;

	myEntity->ResolveDependencies(*this);
	myEntity->Initialize();
}
