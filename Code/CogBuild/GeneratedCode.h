#pragma once
#include "CogType.h"
#include "GeneratedFunction.h"

class GroupingWordReader;
class CogClass;

class GeneratedCode
{
public:
	GeneratedCode(StringView aMainFileName);

	void WriteFiles(const StringView aOutputDirectory);

	CogClass& AddCogClass(String aTypeName, i32 aGeneratedBodyLineIndex);

	void SetShouldGenerateCode(const bool aShouldGenerateCode) { myShouldGenerateCode = true; }
	bool ShouldGenerateCode() const { return myShouldGenerateCode; }

private:
	Map<String, u8> mySourceFileIncludes;
	Array<GeneratedFunction> myGlobalFunctions;
	Array<UniquePtr<CogType>> myDeclaredCogTypes;
	String myGeneratedHeaderFileName;
	String myGeneratedSourceFileName;
	bool myShouldGenerateCode = false;
};
