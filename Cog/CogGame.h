#pragma once
#include "EventListBase.h"
#include "ObjectFunctionView.h"
#include "ThreadID.h"

class ThreadPool;
class ObjectFactory;
class BaseComponentFactory;
class Component;
class Object;
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

	ObjectInitializer CreateObject();

	static CogGame& GetCogGame()
	{
		return *ourGame;
	}

protected:
	virtual void Tick(const Time& aDeltaTime);

	template <typename T>
	void RegisterComponents()
	{
		T* list = new T();
		list->BuildList();
		AssignComponentList(*list);
	}

private:
	void AssignComponentList(const ComponentList& aComponents);
	
	void DispatchTick(Time aDeltaTime);
	void DispatchDraw(RenderTarget& aRenderTarget);

	friend Object;

	// Only to be used by Object::CreateChild, use CreateObject instead
	Object& AllocateObject();

	ObjectFactory& myObjectFactory;
	Array<BaseComponentFactory*> myComponentFactories;

	EventListBase<ObjectFunctionView<void()>> mySynchronizedCallbacks;
	ThreadPool& myThreadPool;
	const ThreadID& myGameThreadID;

	const ComponentList* myComponentList = nullptr;

	static CogGame* ourGame;
};

template <typename TGameType = CogGame>
static TGameType& GetGame()
{
	return CastChecked<TGameType>(CogGame::GetCogGame());
}
