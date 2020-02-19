#pragma once
#include "CogTypeBase.h"
#include "Singleton.generated.h"

class Program;

COGTYPE()
class Singleton : public CogTypeBase
{
	GENERATED_BODY;

protected:
	// Calls Starting, ShuttingDown and SetOwningProgramContext
	friend class ProgramContext;

	virtual bool Starting();
	virtual void ShuttingDown();

	FORCEINLINE ProgramContext& GetProgramContext() const final { return *GetOwningProgramContext(); }

private:
	COGPROPERTY(ProgramContext* OwningProgramContext);
	COGPROPERTY(bool IsBaseCalled = false);
};
