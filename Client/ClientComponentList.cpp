#include "pch.h"
#include "ClientComponentList.h"
#include <ComponentFactory.h>
#include "ClientTestComponent.h"

void ClientComponentList::RegisterComponents()
{
	Base::RegisterComponents();

	DECLARE_COMPONENT_SPECIALIZATION(TestComponent, ClientTestComponent);
}
