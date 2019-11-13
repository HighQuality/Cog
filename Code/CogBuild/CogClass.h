#pragma once
#include "CogType.h"
#include "GeneratedFunction.h"

class CogClass : public CogType
{
public:
	using Base = CogType;

	CogClass(String aClassName, String aBaseClassName, i32 aGeneratedBodyLineIndex);

	Array<String> GenerateGeneratedBodyContents(StringView aGeneratedHeaderIdentifier) const;

	String GenerateHeaderFileContents(const DocumentTemplates& aTemplates, StringView aGeneratedHeaderIdentifier) const override;
	String GenerateSourceFileContents(const DocumentTemplates& aTemplates) const override;

private:
	Array<GeneratedFunction> myGeneratedFunctions;
	i32 myGeneratedBodyLineIndex = -1;
};

