#include "pch.h"
#include "ClientGameComponentList.h"
#include <Client/RenderTarget.h>

void ClientGameComponentList::RegisterComponents()
{
	Base::RegisterComponents();

	DECLARE_COMPONENT(RenderTarget);
}
