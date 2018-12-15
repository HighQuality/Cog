#include "pch.h"
#include "ClientGameComponentList.h"
#include <ComponentFactory.h>
#include "RenderTarget.h"

void ClientGameComponentList::RegisterComponents()
{
	Base::RegisterComponents();

	DECLARE_COMPONENT(RenderTarget);
}
