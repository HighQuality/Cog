#pragma once
#include "CogTypeBase.h"
#include "Singleton.generated.h"

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

	template <typename T>
	FORCEINLINE T& GetProgram() const
	{
		return CheckedCast<T>(GetProgram());
	}

private:
	COGPROPERTY(ProgramContext* OwningProgramContext);
	COGPROPERTY(bool IsBaseCalled = false);
};
