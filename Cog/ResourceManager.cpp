#include "pch.h"
#include "ResourceManager.h"
#include <ThreadPool.h>

ResourceManager::ResourceManager()
{
	// Create 8 threads for loading files
	myThreadPool = new ThreadPool(8);
}

ResourceManager::~ResourceManager()
{
	delete myThreadPool;
	myThreadPool = nullptr;
}

struct CallbackData
{
	Array<u8> data;
	Array<ObjectFunctionView<void(const ArrayView<u8>&, BinaryData&)>> callbacks;
};

void ResourceManager::Tick()
{
	for (const Function<void()>& loader : myScheduledLoads.Gather())
		loader();

	DistributeFinishedActions();
}

void ResourceManager::DistributeFinishedActions()
{
	std::unique_lock<std::mutex> lck(myFileLoadMutex);

	for (const auto& callback : myFinishedActionDistributers)
		callback();
}

CogGame& ResourceManager::GetCogGame()
{
	return GetGame();
}
