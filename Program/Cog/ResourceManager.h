#pragma once
#include <Utilities/Function.h>
#include <Utilities/BinaryData.h>
#include <Utilities/ThreadPool.h>
#include <Utilities/EventList.h>
#include "Resource.h"

class Resource;
class ThreadPool;
class CogGame;

class ResourceManager : public Object
{
public:
	using Base = Object;

	ResourceManager();
	~ResourceManager();

	template <typename TResourceType>
	void Load(const StringView& aResourcePath, const ObjectFunctionView<void(TResourceType)>& aFunctionCallback)
	{
		CHECK(aFunctionCallback.IsValid());

		std::unique_lock<std::mutex> lck(myMutex);

		if (Ptr<Resource>* alreadyLoadedResource = myLoadedResources.Find(aResourcePath))
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

		myScheduledLoads.Submit([this, resourcePath, aFunctionCallback]()
		{
			auto callback = [aFunctionCallback](Resource& aResource)
			{
				aFunctionCallback.TryCall(static_cast<TResourceType&>(aResource));
			};

			if (Ptr<Resource>* alreadyLoadedResource = myLoadedResources.Find(resourcePath.View()))
			{
				if (CheckResourcePtrValid(*alreadyLoadedResource))
				{
					Resource& resource = **alreadyLoadedResource;
					resource.RegisterCallback(callback);
					return;
				}
			}

			TResourceType& resource = GetCogGame().template CreateObject<TResourceType>();
			myLoadedResources.Add(resourcePath, resource);
			resource.RegisterCallback(callback);
			resource.BeginLoad(resourcePath);
		});
	}

	void Tick();

private:
	static bool CheckResourcePtrValid(const Ptr<Resource>& ResourcePtr);

	static CogGame& GetCogGame();

	EventList<Function<void()>> myScheduledLoads;
	Map<String, Ptr<Resource>> myLoadedResources;

	std::mutex myMutex;
};
