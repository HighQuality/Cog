#include "pch.h"
#include "ObjectInitializer.h"
#include "Object.h"

void ObjectInitializer::InitializeObject()
{
	myObject->ResolveDependencies();
	myObject->Initialize();
}
