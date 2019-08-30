#include "pch.h"
#include "ClientTypeList.h"

#include "SpriteComponent.h"
#include "ClientTestComponent.h"

void ClientTypeList::RegisterTypes()
{
	Base::RegisterComponents();

	REGISTER_TYPE(SpriteComponent);
	
	REGISTER_TYPE_SPECIALIZATION(TestComponent, ClientTestComponent);
}
