#include "CogBuildPch.h"
#include "GeneratedCode.h"

GeneratedCode::GeneratedCode(const StringView aMainFileName)
{
	myFileName = Format(L"%_generated", aMainFileName);
}

void GeneratedCode::WriteFiles(const StringView aOutputDirectory)
{
	
}
