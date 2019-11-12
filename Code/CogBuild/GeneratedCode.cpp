#include "CogBuildPch.h"
#include "GeneratedCode.h"
#include "CogClass.h"

GeneratedCode::GeneratedCode(const StringView aMainFileName)
{
	myGeneratedHeaderFileName = Format(L"%_generated.h", aMainFileName);
	myGeneratedSourceFileName = Format(L"%_generated.cpp", aMainFileName);
}

void GeneratedCode::WriteFiles(const StringView aOutputDirectory)
{
	Println(L"", aOutputDirectory, myGeneratedHeaderFileName);
}

CogClass& GeneratedCode::AddCogClass(String aTypeName, const i32 aGeneratedBodyLineIndex)
{
	CogClass& type = *new CogClass(Move(aTypeName), aGeneratedBodyLineIndex);
	// UniquePtr takes ownership of CogClass allocation
	myDeclaredCogTypes.Add(UniquePtr<CogType>(&type));
	return type;
}
 