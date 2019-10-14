#include "pch.h"
#include "EngineTypeList.h"

#include "ResourceManager.h"

void EngineTypeList::RegisterTypes()
{
	Base::RegisterTypes();

	REGISTER_TYPE(ResourceManager);
}
