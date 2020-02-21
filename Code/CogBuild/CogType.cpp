#include "CogBuildPch.h"
#include "CogType.h"
#include "CogBuildUtilities.h"

CogType::CogType(String aAbsoluteFilePath, const i32 aDeclarationLine, String aTypeName, String aBaseTypeName)
{
	myAbsoluteFilePath = Move(aAbsoluteFilePath);
	myTypeName = Move(aTypeName);
	myBaseTypeName = Move(aBaseTypeName);
	myDeclarationLine = aDeclarationLine;
}

bool CogType::ResolveDependencies(const Map<String, CogType*>& aCogTypes)
{
	if (myBaseTypeName.View())
	{
		if (CogType* baseType = aCogTypes.Find(myBaseTypeName.View(), nullptr))
		{
			myBaseType = baseType;
		}
		else
		{
			ReportErrorInFile(Format(L"Can't find base type \"%\"", myBaseTypeName).View(), myAbsoluteFilePath, myDeclarationLine);
			return false;
		}
	}

	return true;
}

void CogType::PostResolveDependencies()
{
}

Array<String> CogType::GenerateHeaderFileContents(const DocumentTemplates& aTemplates, const StringView aGeneratedHeaderIdentifier) const
{
	return Array<String>();
}

Array<String> CogType::GenerateSourceFileContents(const DocumentTemplates& aTemplates) const
{
	return Array<String>();
}

String CogType::GetDeclarationLocation() const
{
	return Format(L"%(%)", myAbsoluteFilePath, myDeclarationLine);
}
