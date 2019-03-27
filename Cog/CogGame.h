#pragma once
#include "EventList.h"
#include "ObjectFunctionView.h"
#include "ThreadID.h"
#include "ObjectFactory.h"
#include "ResourceManager.h"

class ThreadPool;
class EntityFactory;
class BaseComponentFactory;
class Component;
class Entity;
class ComponentList;
class Widget;
struct FrameData;

class CogGame
{
public:
	CogGame();
	virtual ~CogGame();

	DELETE_COPYCONSTRUCTORS_AND_MOVES(CogGame);

	virtual bool ShouldKeepRunning() const = 0;

	virtual void Run();

	BaseComponentFactory& FindOrCreateComponentFactory(TypeID<Component> aComponentType);

	EntityInitializer CreateEntity();

	template <typename T>
	T& CreateObject()
	{
		const auto objectTypeID = TypeID<Object>::Resolve<T>();
		
		BaseObjectFactory& factory = FindOrCreateObjectFactory(objectTypeID, [&objectTypeID]() { return new ObjectFactory<T>(objectTypeID); });

		T& object = CastChecked<T>(factory.AllocateGeneric());

		if constexpr(IsDerivedFrom<T, Widget>)
		{
			NewWidgetCreated(object);
		}

		return object;
	}

	ResourceManager& GetResourceManager() const
	{
		return *myResourceManager;
	}
	
	static CogGame& GetCogGame()
	{
		return *ourGame;
	}

	bool IsInGameThread() const
	{
		return myGameThreadID == ThreadID::Get();
	}

protected:
	virtual void SynchronizedTick(const Time& aDeltaTime);

	virtual void QueueDispatchers(const Time& aDeltaTime);
	virtual void DispatchTick();

	virtual void NewWidgetCreated(Widget& aWidget) = 0;

	template <typename T>
	void RegisterComponents()
	{
		T* list = new T();
		list->BuildList();
		AssignComponentList(*list);
	}

	BaseObjectFactory& FindOrCreateObjectFactory(const TypeID<Object>& aObjectType, const FunctionView<BaseObjectFactory*()>& aFactoryCreator);

	Array<BaseComponentFactory*> myComponentFactories;
	
	FrameData* myFrameData = nullptr;

private:
	void CreateResourceManager();
	void AssignComponentList(const ComponentList& aComponents);
	virtual void UpdateFrameData(FrameData& aData, const Time& aDeltaTime);

	friend Entity;

	// Only to be used by Entity::CreateChild, use CreateEntity instead
	Entity& AllocateEntity();

	EntityFactory* myEntityFactory;
	Array<BaseObjectFactory*> myObjectFactories;

	const ThreadID& myGameThreadID;

	const ComponentList* myComponentList = nullptr;

	Ptr<ResourceManager> myResourceManager;

	static CogGame* ourGame;
};

template <typename TGameType = CogGame>
TGameType& GetGame()
{
	return CastChecked<TGameType>(CogGame::GetCogGame());
}

inline ResourceManager& GetResourceManager()
{
	return GetGame().GetResourceManager();
}
