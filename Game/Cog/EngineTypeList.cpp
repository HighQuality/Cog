#include "pch.h"
#include "EngineTypeList.h"

#include "CogGame.h"
#include "ResourceManager.h"

void EngineTypeList::RegisterTypes()
{
	Base::RegisterTypes();

	REGISTER_TYPE(CogGame);
	REGISTER_TYPE(ResourceManager);
}
