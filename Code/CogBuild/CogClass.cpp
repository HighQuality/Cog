#include "CogBuildPch.h"
#include "CogClass.h"

CogClass::CogClass(String aClassName, i32 aGeneratedBodyLineIndex)
	: Base(Move(aClassName))
{
	myGeneratedBodyLineIndex = aGeneratedBodyLineIndex;
}

Array<String> CogClass::GenerateGeneratedBodyContents() const
{
	Array<String> generatedLines;
	
	return generatedLines;
}
