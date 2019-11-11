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

	CogClass& AddCogClass(StringView aTypeName, i32 aGeneratedBodyLineIndex);

private:
	Map<String, u8> mySourceFileIncludes;
	Array<GeneratedFunction> myGlobalFunctions;
	Array<UniquePtr<CogType>> myDeclaredCogTypes;
	String myFileName;
};
