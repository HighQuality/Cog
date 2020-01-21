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
	for (const Function<void()>& loader : ScheduledLoads().Gather())
		loader();
}

bool ResourceManager::CheckResourcePtrValid(const Ptr<Resource>& ResourcePtr)
{
	return ResourcePtr.IsValid();
}

CogGame& ResourceManager::GetCogGame()
{
	return GetGame();
}
