#pragma once
#include "CogType.h"
#include "GeneratedFunction.h"

class CogClass : public CogType
{
public:
	using Base = CogType;

	CogClass(String aClassName, i32 aGeneratedBodyLineIndex);

	Array<String> GenerateGeneratedBodyContents() const;

private:
	Array<GeneratedFunction> myGeneratedFunctions;
	i32 myGeneratedBodyLineIndex = -1;
};

