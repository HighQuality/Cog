#include "CogPch.h"
#include "CogTypeBase.h"
#include "ProgramContext.h"

Singleton& CogTypeBase::GetSingleton(const TypeID<CogTypeBase>& aTypeId) const
{
	return GetProgramContext().GetSingleton(aTypeId);
}
