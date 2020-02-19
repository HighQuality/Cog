#include "CogPch.h"
#include "ResourceManager.h"
#include <Cog\CogGame.h>

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

void ResourceManager::Tick()
{
	for (const Function<void()>& loader : GetScheduledLoads().Gather())
		loader();
}

bool ResourceManager::CheckResourcePtrValid(const Ptr<Resource>& ResourcePtr)
{
	return ResourcePtr.IsValid();
}
