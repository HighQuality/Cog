#pragma once
#include <Threading/ThreadID.h>
#include "ObjectFunctionView.h"
#include "Program.h"
#include "CogGame.generated.h"

class MessageSystem;
struct FrameData;

COGTYPE(Specialization)
class CogGame : public Program
{
	GENERATED_BODY;

protected:
	bool Starting() override;
	void ShuttingDown() override;

	void SynchronizedTick(const Time& aDeltaTime) override;

private:
	virtual void UpdateFrameData(FrameData& aData, const Time& aDeltaTime);

	COGPROPERTY(UniquePtr<FrameData> FrameData);
};
