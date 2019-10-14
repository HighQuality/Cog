#pragma once
#include <Memory/Factory.h>
#include <Threading/ThreadID.h>
#include "ObjectFunctionView.h"
#include "ResourceManager.h"

template <typename TSignature>
class FunctionView;

template <typename TReturn, typename... TArgs>
class FunctionView<TReturn(TArgs...)>;

class MessageSystem;
class BaseFactory;
class ThreadPool;
class Object;
class TypeList;
struct FrameData;

class CogGame
{
public:
	CogGame();
	virtual ~CogGame();

	DELETE_COPYCONSTRUCTORS_AND_MOVES(CogGame);

	virtual bool ShouldKeepRunning() const = 0;

	virtual void Run();

	Object& CreateObjectByType(TypeID<Object> aType);

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

	template <typename T>
	void RegisterTypes()
	{
		UniquePtr<T> list = MakeUnique<T>();
		list->BuildList();
		AssignTypeList(Move(list));
	}

	BaseFactory& FindOrCreateObjectFactory(const TypeID<Object>& aObjectType, const FunctionView<BaseFactory*()>& aFactoryCreator);

	UniquePtr<FrameData> myFrameData;

private:
	void CreateResourceManager();
	void AssignTypeList(UniquePtr<const TypeList> aTypeList);
	virtual void UpdateFrameData(FrameData& aData, const Time& aDeltaTime);
	void TickDestroys();

	friend Object;
	friend Object;

	void ScheduleDestruction(Object& aObject);

	Array<BaseFactory*> myObjectFactories;
	UniquePtr<MessageSystem> myMessageSystem;

	const ThreadID& myGameThreadID;

	UniquePtr<const TypeList> myTypeList;

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
