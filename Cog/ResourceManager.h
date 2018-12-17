#pragma once
#include <Function.h>

class Resource;
class ThreadPool;

class ResourceManager : public Object
{
public:
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
			TResourceType& resource = GetGame().CreateObject<TResourceType>();
			myLoadedResources.Add(aResourcePath, resource);
			resource.RegisterCallback(Move(aFunctionCallback));
			resource.BeginLoad();
		}
		else
		{
			myScheduledLoads.Submit([this, aResourcePath, aFunctionCallback]()
			{
				Load<TResourceType>(aResourcePath, Move(aFunctionCallback));
			});
		}
	}

	void LoadScheduledResources();

private:
	ThreadPool* myThreadPool = nullptr;
	EventList<Function<void()>> myScheduledLoads;
	Map<String, Ptr<Resource>> myLoadedResources;
};
