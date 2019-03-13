#pragma once
#include "EventList.h"
#include "ObjectFunctionView.h"
#include "ThreadID.h"
#include "ObjectFactory.h"
#include "ResourceManager.h"
#include "FileLoader.h"

class ThreadPool;
class EntityFactory;
class BaseComponentFactory;
class Component;
class Entity;
class ComponentList;
class Widget;

class CogGame
{
public:
	CogGame();
	virtual ~CogGame();

	CogGame(const CogGame&) = delete;
	CogGame& operator=(const CogGame&) = delete;
	
	CogGame(CogGame&&) = delete;
	CogGame& operator=(CogGame&&) = delete;
	
	virtual bool ShouldKeepRunning() const = 0;

	virtual void Run();

	template <typename TType, typename ...TArgs>
	void Synchronize(TType& aObject, void(TType::*aFunction)(TArgs...))
	{
		mySynchronizedCallbacks.Submit(ObjectFunctionView<void(TArgs...)>(aObject, aFunction));
	}

	FORCEINLINE bool IsInGameThread() const { return myGameThreadID == ThreadID::Get(); }

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
	
	FileLoader& GetFileLoader() const
	{
		return *myFileLoader;
	}

	static CogGame& GetCogGame()
	{
		return *ourGame;
	}

protected:
	virtual void Tick(const Time& aDeltaTime);

	virtual void DispatchWork(const Time& aDeltaTime);
	virtual void DispatchTick(const Time& aDeltaTime);

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
	ThreadPool& myThreadPool;

private:
	void CreateResourceManager();
	void CreateFileLoader();
	void AssignComponentList(const ComponentList& aComponents);
	
	friend Entity;

	// Only to be used by Entity::CreateChild, use CreateEntity instead
	Entity& AllocateEntity();

	EntityFactory& myEntityFactory;
	Array<BaseObjectFactory*> myObjectFactories;

	EventList<ObjectFunctionView<void()>> mySynchronizedCallbacks;
	const ThreadID& myGameThreadID;

	const ComponentList* myComponentList = nullptr;

	Ptr<ResourceManager> myResourceManager;
	Ptr<FileLoader> myFileLoader;

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

inline FileLoader& GetFileLoader()
{
	return GetGame().GetFileLoader();
}
