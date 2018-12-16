#pragma once
#include "EventList.h"
#include "ObjectFunctionView.h"
#include "ThreadID.h"
#include "WidgetFactory.h"

class ThreadPool;
class EntityFactory;
class BaseComponentFactory;
class Component;
class Entity;
class ComponentList;

class CogGame
{
public:
	CogGame();
	virtual ~CogGame();
	
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
	T& CreateWidget()
	{
		const auto widgetTypeID = TypeID<Widget>::Resolve<T>();
		BaseWidgetFactory& factory = FindOrCreateWidgetFactory(widgetTypeID, [&widgetTypeID]() { return new WidgetFactory<T>(widgetTypeID); });
		return CastChecked<T>(factory.AllocateGeneric());
	}

	static CogGame& GetCogGame()
	{
		return *ourGame;
	}

protected:
	virtual void Tick(const Time& aDeltaTime);

	virtual void DispatchWork(const Time& aDeltaTime);
	virtual void DispatchTick(const Time& aDeltaTime);

	template <typename T>
	void RegisterComponents()
	{
		T* list = new T();
		list->BuildList();
		AssignComponentList(*list);
	}

	virtual BaseWidgetFactory& FindOrCreateWidgetFactory(const TypeID<Widget>& aWidgetType, const FunctionView<BaseWidgetFactory*()>& aFactoryCreator) = 0;

	Array<BaseComponentFactory*> myComponentFactories;
	ThreadPool& myThreadPool;

private:
	void AssignComponentList(const ComponentList& aComponents);
	
	friend Entity;

	// Only to be used by Entity::CreateChild, use CreateEntity instead
	Entity& AllocateEntity();

	EntityFactory& myEntityFactory;

	EventList<ObjectFunctionView<void()>> mySynchronizedCallbacks;
	const ThreadID& myGameThreadID;

	const ComponentList* myComponentList = nullptr;

	static CogGame* ourGame;
};

template <typename TGameType = CogGame>
static TGameType& GetGame()
{
	return CastChecked<TGameType>(CogGame::GetCogGame());
}
