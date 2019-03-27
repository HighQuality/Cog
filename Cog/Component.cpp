#include "pch.h"
#include "Component.h"

void Component::Destroy()
{
	FATAL(L"You may not call Destroy directly on components, only their owning entity may be destroyed");
}
