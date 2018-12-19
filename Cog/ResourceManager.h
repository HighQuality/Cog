#pragma once
#include <Function.h>
#include <BinaryData.h>
#include <ThreadPool.h>

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
	void Load(const StringView& aResourcePath, ObjectFunctionView<void(TResourceType)> aFunctionCallback)
	{
		CHECK(aFunctionCallback.IsValid());

		if (Ptr<Resource>* alreadyLoadedResource = myLoadedResources.Find(aResourcePath))
		{
			if (alreadyLoadedResource)
			{
				Resource& genericResource = **alreadyLoadedResource;
				TResourceType& resource = CastChecked<TResourceType>(genericResource);
				resource.RegisterCallback(Move(aFunctionCallback));
				aFunctionCallback(resource);
				return;
			}
		}

		if (IsInGameThread())
		{
			TResourceType& resource = GetCogGame().template CreateObject<TResourceType>();
			myLoadedResources.Add(aResourcePath, resource);
			resource.RegisterCallback(Move(aFunctionCallback));
			resource.BeginLoad(aResourcePath);
		}
		else
		{
			myScheduledLoads.Submit([this, aResourcePath, aFunctionCallback]()
			{
				Load<TResourceType>(aResourcePath, Move(aFunctionCallback));
			});
		}
	}

	void Tick();

private:
	friend Resource;

	void DistributeFinishedActions();
	
	template <typename TReturn>
	void DoLoadAction(Function<TReturn()> aWork, ObjectFunctionView<void(TReturn)> aCallback)
	{
		myThreadPool->QueueSingle([this, aWork, aCallback]
		{
			TReturn result = aWork();

			std::unique_lock<std::mutex> lck(myFileLoadMutex);
			// TODO: Result is copied...
			myFinishedActionDistributers.Add(Function<void()>([result, aCallback]
			{
				// TODO: And again...
				aCallback.TryCall(result);
			}));
		});
	}

	static CogGame& GetCogGame();

	ThreadPool* myThreadPool = nullptr;
	EventList<Function<void()>> myScheduledLoads;
	Map<String, Ptr<Resource>> myLoadedResources;

	std::mutex myFileLoadMutex;
	Array<Function<void()>> myFinishedActionDistributers;
};
