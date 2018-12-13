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
		mySynchronizedCallbacks.Submit(ObjectFunctionView<void(TArgs...)>(aObject, aFunction));
	}

	FORCEINLINE bool IsInGameThread() const { return myGameThreadID == ThreadID::Get(); }

	static CogGame& Get()
	{
		return *ourGame;
	}

protected:
	virtual void Tick(const Time& aDeltaTime);

	void AddScene(CogScene& aWorld);

private:
	EventListBase<ObjectFunctionView<void()>> mySynchronizedCallbacks;
	ThreadPool& myThreadPool;
	Array<CogScene*> myWorlds;
	const ThreadID& myGameThreadID;

	static CogGame* ourGame;
};
