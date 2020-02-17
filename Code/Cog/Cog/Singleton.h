#pragma once
#include "CogTypeBase.h"
#include "Singleton.generated.h"

COGTYPE()
class Singleton : public CogTypeBase
{
	GENERATED_BODY;

protected:
	virtual bool Starting();
	virtual void ShuttingDown();

private:
	COGPROPERTY(bool IsBaseCalled = false);
};
