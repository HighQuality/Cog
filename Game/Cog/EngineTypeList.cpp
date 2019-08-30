#include "pch.h"
#include "EngineTypeList.h"
#include "TestComponent.h"

void EngineTypeList::RegisterTypes()
{
	Base::RegisterTypes();

	REGISTER_TYPE(TestComponent);
}
