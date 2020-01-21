#pragma once
#include <Function/Function.h>
#include <Containers/BinaryData.h>
#include <Threading/ThreadPool/ThreadPool.h>
#include <Containers/EventList.h>
#include "Resource.h"
#include "ResourceManager.generated.h"

class Resource;
class ThreadPool;
class CogGame;

COGTYPE()
class ResourceManager : public Object
{
	GENERATED_BODY;

public:
	ResourceManager();
	~ResourceManager();

	template <typename TResourceType>
	void Load(const StringView& aResourcePath, const ObjectFunctionView<void(TResourceType)>& aFunctionCallback)
	{
		CHECK(aFunctionCallback.IsValid());

		std::unique_lock<std::mutex> lck(GetMutexRef());

		auto& loadedResources = GetLoadedResourcesRef();

		if (Ptr<Resource>* alreadyLoadedResource = loadedResources.Find(aResourcePath))
		{
			if (alreadyLoadedResource->IsValid())
			{
				Resource& genericResource = **alreadyLoadedResource;
				lck.unlock();
				
				TResourceType& resource = CastChecked<TResourceType>(genericResource);
				aFunctionCallback(resource);
				return;
			}
		}
			
		String resourcePath(aResourcePath);

		GetScheduledLoadsRef().Submit([&loadedResources, resourcePath, aFunctionCallback]()
		{
			auto callback = [aFunctionCallback](Resource& aResource)
			{
				aFunctionCallback.TryCall(static_cast<TResourceType&>(aResource));
			};

			if (Ptr<Resource>* alreadyLoadedResource = loadedResources.Find(resourcePath.View()))
			{
				if (CheckResourcePtrValid(*alreadyLoadedResource))
				{
					Resource& resource = **alreadyLoadedResource;
					resource.RegisterCallback(callback);
					return;
				}
			}

			TResourceType& resource = GetCogGame().template CreateObject<TResourceType>();
			loadedResources.Add(resourcePath, resource);
			resource.RegisterCallback(callback);
			resource.BeginLoad(resourcePath);
		});
	}

	void Tick();

private:
	static bool CheckResourcePtrValid(const Ptr<Resource>& ResourcePtr);

	static CogGame& GetCogGame();

	COGPROPERTY(EventList<Function<void()>> ScheduledLoads);
	COGPROPERTY(Map<String, Ptr<Resource>> LoadedResources);

	COGPROPERTY(std::mutex Mutex);
};
