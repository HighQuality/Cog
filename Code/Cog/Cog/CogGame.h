#pragma once
#include <Threading/ThreadID.h>
#include "ObjectFunctionView.h"
#include "ResourceManager.h"
#include "CogGame.generated.h"

template <typename TSignature>
class FunctionView;

template <typename TReturn, typename... TArgs>
class FunctionView<TReturn(TArgs...)>;

class MessageSystem;
class Object;
struct FrameData;

COGTYPE()
class CogGame : public Object
{
	GENERATED_BODY;

public:
	CogGame();
	virtual ~CogGame();

	DELETE_COPYCONSTRUCTORS_AND_MOVES(CogGame);

	virtual bool ShouldKeepRunning() const = 0;

	virtual void Run();

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

	UniquePtr<FrameData> myFrameData;

private:
	void CreateResourceManager();
	virtual void UpdateFrameData(FrameData& aData, const Time& aDeltaTime);
	void TickDestroys();

	friend Object;

	void ScheduleDestruction(Object& aObject);

	UniquePtr<MessageSystem> myMessageSystem;

	const ThreadID& myGameThreadID;
	
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
