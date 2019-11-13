#include "CogBuildPch.h"
#include "CogType.h"

CogType::CogType(String aTypeName)
{
	myTypeName = Move(aTypeName);
}

String CogType::GenerateHeaderFileContents() const
{
	return String();
}

String CogType::GenerateSourceFileContents() const
{
	return String();
}
