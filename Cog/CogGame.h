#pragma once
#include "EventListBase.h"
#include "ObjectFunctionView.h"
#include "ThreadID.h"

class ThreadPool;

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
		mySynchronizedCallbacks.Submit(new ObjectFunctionView(aObject, aFunction));
	}

	FORCEINLINE bool IsInGameThread() const { return myGameThreadID == ThreadID::Get(); }

	static CogGame& Get()
	{
		return *ourGame;
	}

protected:
	virtual void Tick(const Time& aDeltaTime);

	void AddWorld(CogGameWorld& aWorld);

private:
	EventListBase<BaseObjectFunctionView<void()>*> mySynchronizedCallbacks;
	ThreadPool& myThreadPool;
	Array<CogGameWorld*> myWorlds;
	const ThreadID& myGameThreadID;

	static CogGame* ourGame;
};
