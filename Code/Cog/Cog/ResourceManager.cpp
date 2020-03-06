#include "CogPch.h"
#include "ResourceManager.h"

bool ResourceManager::Starting()
{
	if (!Base::Starting())
		return false;

	return true;
}

void ResourceManager::ShuttingDown()
{
	Base::ShuttingDown();
}
