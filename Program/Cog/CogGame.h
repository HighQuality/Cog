#pragma once
#include <Utilities/Factory.h>
#include <Utilities/ThreadID.h>
#include "ObjectFunctionView.h"
#include "ResourceManager.h"

template <typename TSignature>
class FunctionView;

template <typename TReturn, typename... TArgs>
class FunctionView<TReturn(TArgs...)>;

template <typename T>
class ComponentFactory;

class MessageSystem;
class BaseFactory;
class ThreadPool;
class Component;
class Entity;
class Object;
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

	template <typename T>
	ComponentFactory<T>& FindOrCreateComponentFactory()
	{
		return static_cast<ComponentFactory<T>&>(FindOrCreateComponentFactory(TypeID<Component>::Resolve<T>()));
	}

	BaseComponentFactory& FindOrCreateComponentFactory(TypeID<Component> aComponentType);

	EntityInitializer CreateEntity();

	template <typename T>
	T& CreateObject()
	{
		static_assert(!IsDerivedFrom<T, Entity>, L"Use CreateEntity to create entities!");
		static_assert(!IsDerivedFrom<T, Component>, L"Components may only be created on entities!");

		const auto objectTypeID = TypeID<Object>::Resolve<T>();
		
		BaseFactory& factory = FindOrCreateObjectFactory(objectTypeID, [&objectTypeID]() { return new Factory<T>(); });

		T& object = *static_cast<T*>(factory.AllocateRawObject());

		if constexpr(IsDerivedFrom<T, Widget>)
			NewWidgetCreated(object);

		return object;
	}

	ResourceManager& GetResourceManager() const
	{
		return *myResourceManager;
	}
	
	FORCEINLINE static CogGame& GetCogGame()
	{
		return *ourGame;
	}

	bool IsInGameThread() const
	{
		return myGameThreadID == ThreadID::Get();
	}

	FORCEINLINE const FrameData& GetFrameData() const { return *myFrameData; }

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

	BaseFactory& FindOrCreateObjectFactory(const TypeID<Object>& aObjectType, const FunctionView<BaseFactory*()>& aFactoryCreator);

	Array<BaseComponentFactory*> myComponentFactories;
	
	FrameData* myFrameData = nullptr;

private:
	void CreateResourceManager();
	void AssignComponentList(const ComponentList& aComponents);
	virtual void UpdateFrameData(FrameData& aData, const Time& aDeltaTime);
	void TickDestroys();

	friend Entity;
	friend Object;

	void ScheduleDestruction(Object& aObject);

	// Only to be used by Entity::CreateChild and CreateEntity, use CreateEntity instead
	Entity& AllocateEntity();

	BaseFactory* myEntityFactory;
	Array<BaseFactory*> myObjectFactories;
	MessageSystem* myMessageSystem;

	const ThreadID& myGameThreadID;

	const ComponentList* myComponentList = nullptr;

	Ptr<ResourceManager> myResourceManager;

	std::mutex myDestroyMutex;
	Array<Array<Object*>> myScheduledDestroys;


	static CogGame* ourGame;
};

template <typename TGameType = CogGame>
FORCEINLINE TGameType& GetGame()
{
	return CastChecked<TGameType>(CogGame::GetCogGame());
}

inline ResourceManager& GetResourceManager()
{
	return GetGame().GetResourceManager();
}
