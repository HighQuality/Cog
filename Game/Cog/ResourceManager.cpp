#include "pch.h"
#include "ResourceManager.h"

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

bool ResourceManager::CheckResourcePtrValid(const Ptr<Resource>& ResourcePtr)
{
	return ResourcePtr.IsValid();
}

CogGame& ResourceManager::GetCogGame()
{
	return GetGame();
}