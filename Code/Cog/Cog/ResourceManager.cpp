#include "CogPch.h"
#include "ResourceManager.h"
#include <Cog\CogGame.h>

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
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
