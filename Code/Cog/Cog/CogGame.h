#pragma once
#include <Threading/ThreadID.h>
#include "ObjectFunctionView.h"
#include "Program.h"
#include "CogGame.generated.h"

class MessageSystem;

COGTYPE(Specialization)
class CogGame : public Program
{
	GENERATED_BODY;

protected:
	bool Starting() override;
	void ShuttingDown() override;

	void SynchronizedTick(f32 aDeltaSeconds) override;
};
