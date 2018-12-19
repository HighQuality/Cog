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
	Array<ObjectFunctionView<BinaryData(const ArrayView<u8>&)>> callbacks;
};

void ResourceManager::LoadScheduledResources()
{
	for (const Function<void()>& loader : myScheduledLoads.Gather())
		loader();

	Array<CallbackData> callbacks;

	{
		std::unique_lock<std::mutex> lck(myFileLoadMutex);
		for (const auto& keyValuePair : myLoadedFiles)
		{
			const StringView path = keyValuePair.key;
			const Array<u8>& fileData = keyValuePair.value;

			if (auto* callbackList = myFileCallbacks.Find(path))
			{
				CallbackData data;
				// Intentional copy since callback can load new files changing callback array to reallocate
				data.callbacks = *callbackList;
				TODO_ // Make sure fileData isn't copied so we don't duplicate the file in ram, also make Move() not compiled when passed const-ref
				data.data = Move(fileData);
				callbacks.Add(data);
			}
		}
		myLoadedFiles.Clear();
	}

	for (const CallbackData& data : callbacks)
	{
		for (const auto& callback : data.callbacks)
			callback.TryCall(data.data);
	}
}

void ResourceManager::LoadFile(const StringView& aPath, ObjectFunctionView<BinaryData(const ArrayView<u8>&)> aCallback)
{
	std::unique_lock<std::mutex> lck(myFileLoadMutex);
	
	myFileCallbacks.FindOrAdd(aPath).Add(Move(aCallback));
	
	auto loader = [this, aPath]()
	{
		FILE* f;

		if (_wfopen_s(&f, aPath.GetData(), L"rb"))
			FATAL(L"_wfopen_s failed");

		fseek(f, 0, SEEK_END);
		const long size = ftell(f);
		fseek(f, 0, SEEK_SET);

		Array<u8> data;
		data.Resize(CastBoundsChecked<long>(size));
		fread(data.GetData(), size, 1, f);

		if (fclose(f))
			FATAL(L"fclose failed");

		std::unique_lock<std::mutex> lck(myFileLoadMutex);
		myLoadedFiles.Add(aPath, Move(data));
	};

	lck.unlock();

	myThreadPool->QueueSingle(loader);
}

CogGame& ResourceManager::GetCogGame()
{
	return GetGame();
}
