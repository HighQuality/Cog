#pragma once
#include <Threading/ThreadID.h>
#include "ObjectFunctionView.h"
#include "ResourceManager.h"
#include "Program.h"
#include "CogGame.generated.h"

class MessageSystem;
struct FrameData;

COGTYPE(Specialization)
class CogGame : public Program
{
	GENERATED_BODY;

public:
	CogGame();
	virtual ~CogGame();

	virtual bool ShouldKeepRunning() const = 0;

	virtual void Run();

	ResourceManager& GetResourceManager() const
	{
		return *myResourceManager;
	}
	
	bool IsInGameThread() const;

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

	Ptr<ResourceManager> myResourceManager;

	std::mutex myDestroyMutex;
	Array<Array<Object*>> myScheduledDestroys;
};

inline ResourceManager& GetResourceManager()
{
	return GetGame().GetResourceManager();
}
