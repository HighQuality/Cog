#include "CogBuildPch.h"
#include "CogType.h"

CogType::CogType(String aTypeName)
{
	myTypeName = Move(aTypeName);
}
