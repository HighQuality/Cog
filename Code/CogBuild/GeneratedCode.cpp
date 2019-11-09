#include "CogBuildPch.h"
#include "GeneratedCode.h"

GeneratedCode::GeneratedCode(const StringView aMainFileName)
{
	myFileName = Format(L"%_generated", aMainFileName);
}
