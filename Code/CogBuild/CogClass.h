#pragma once
#include "CogType.h"
#include "GeneratedFunction.h"

class CogClass : public CogType
{
public:
	CogClass();

	Array<String> GenerateGeneratedBodyContents() const;

private:
	Array<GeneratedFunction> myGeneratedFunctions;
};

