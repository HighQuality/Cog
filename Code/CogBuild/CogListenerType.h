#pragma once
#include "CogType.h"

class CogListenerType : public CogType
{
public:
	using Base = CogType;

	CogListenerType(String aAbsoluteFilePath, i32 aDeclarationLine, String aClassName, String aBaseClassName, i32 aGeneratedBodyLineIndex);

	Array<String> GenerateGeneratedBodyContents(StringView aGeneratedHeaderIdentifier) const;

	Array<String> GenerateHeaderFileContents(const DocumentTemplates& aTemplates, StringView aGeneratedHeaderIdentifier) const override;
	Array<String> GenerateSourceFileContents(const DocumentTemplates& aTemplates) const override;

private:
	i32 myGeneratedBodyLineIndex = -1;
};
