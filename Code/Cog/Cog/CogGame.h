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

protected:
	void Created() override;
	void Destroyed() override;

	void SynchronizedTick(const Time& aDeltaTime) override;

private:
	virtual void UpdateFrameData(FrameData& aData, const Time& aDeltaTime);

	COGPROPERTY(UniquePtr<FrameData> FrameData);
	COGPROPERTY(Ptr<ResourceManager> ResourceManager);
};
