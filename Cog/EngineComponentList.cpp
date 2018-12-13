#include "pch.h"
#include "EngineComponentList.h"
#include "ComponentFactory.h"
#include "TestComponent.h"

void EngineComponentList::RegisterComponents()
{
	Base::RegisterComponents();

	DECLARE_COMPONENT(TestComponent);
}
