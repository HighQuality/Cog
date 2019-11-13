#include "CogBuildPch.h"
#include "CogType.h"

CogType::CogType(String aTypeName, String aBaseTypeName)
{
	myTypeName = Move(aTypeName);
	myBaseTypeName = Move(aBaseTypeName);
}

String CogType::GenerateHeaderFileContents(const DocumentTemplates& aTemplates, const StringView aGeneratedHeaderIdentifier) const
{
	return String();
}

String CogType::GenerateSourceFileContents(const DocumentTemplates& aTemplates) const
{
	return String();
}
