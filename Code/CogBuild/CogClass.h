#pragma once
#include "CogType.h"
#include "GeneratedFunction.h"

class CogClass : public CogType
{
public:
	using Base = CogType;

	CogClass(String aClassName, i32 aGeneratedBodyLineIndex);

	Array<String> GenerateGeneratedBodyContents() const;

	String GenerateHeaderFileContents() const override;
	String GenerateSourceFileContents() const override;

private:
	Array<GeneratedFunction> myGeneratedFunctions;
	i32 myGeneratedBodyLineIndex = -1;
};

