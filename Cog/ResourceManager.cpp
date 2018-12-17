#include "pch.h"
#include "ResourceManager.h"
#include <ThreadPool.h>

ResourceManager::ResourceManager()
{
	myThreadPool = new ThreadPool(Max(1, static_cast<i32>(std::thread::hardware_concurrency()) - 2));
}

ResourceManager::~ResourceManager()
{
	delete myThreadPool;
	myThreadPool = nullptr;
}

void ResourceManager::LoadScheduledResources()
{
	for (const Function<void()>& loader : myScheduledLoads.Gather())
		loader();
}
