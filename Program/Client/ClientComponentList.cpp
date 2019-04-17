#include "pch.h"
#include "ClientComponentList.h"

#include "SpriteComponent.h"
#include "ClientTestComponent.h"

void ClientComponentList::RegisterComponents()
{
	Base::RegisterComponents();

	DECLARE_COMPONENT(SpriteComponent);
	
	DECLARE_COMPONENT_SPECIALIZATION(TestComponent, ClientTestComponent);
}
