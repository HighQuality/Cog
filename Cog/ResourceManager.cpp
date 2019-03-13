#include "pch.h"
#include "ResourceManager.h"
#include <ThreadPool.h>

ResourceManager::ResourceManager()
{
}

ResourceManager::~ResourceManager()
{
}

void ResourceManager::Tick()
{
	for (const Function<void()>& loader : myScheduledLoads.Gather())
		loader();
}

CogGame& ResourceManager::GetCogGame()
{
	return GetGame();
}
