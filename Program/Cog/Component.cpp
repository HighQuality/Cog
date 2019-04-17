#include "pch.h"
#include "Component.h"

bool Component::Destroy()
{
	FATAL(L"You may not call Destroy directly on components, only their owning entity may be destroyed");
}
