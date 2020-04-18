#include "CogBuildPch.h"
#include "CogListenerType.h"

CogListenerType::CogListenerType(String aAbsoluteFilePath, i32 aDeclarationLine, String aClassName, String aBaseClassName, i32 aGeneratedBodyLineIndex)
	: Base(Move(aAbsoluteFilePath), aDeclarationLine, Move(aClassName), Move(aBaseClassName))
{
	myGeneratedBodyLineIndex = aGeneratedBodyLineIndex;
}

Array<String> CogListenerType::GenerateGeneratedBodyContents(StringView aGeneratedHeaderIdentifier) const
{
	Array<String> generatedLines;

	generatedLines.Add(Format(L"#define GENERATED_BODY", aGeneratedHeaderIdentifier, myGeneratedBodyLineIndex + 1));
	generatedLines.Add(String(L"public:"));

	generatedLines.Add(Format(L"using Base = %;", GetBaseTypeName()));

	generatedLines.Add(Format(L"static StringView GetStaticTypeName() { return L\"%\"; }", GetTypeName()));
	generatedLines.Add(String(L"const TypeData& GetType() const override;"));

	return generatedLines;
}

Array<String> CogListenerType::GenerateHeaderFileContents(const DocumentTemplates& aTemplates, StringView aGeneratedHeaderIdentifier) const
{
	Array<String> generatedLines = Base::GenerateHeaderFileContents(aTemplates, aGeneratedHeaderIdentifier);

	{
		const Array<String> generatedBodyContents = GenerateGeneratedBodyContents(aGeneratedHeaderIdentifier);

		for (i32 i = 0; i < generatedBodyContents.GetLength(); ++i)
		{
			if (i + 1 == generatedBodyContents.GetLength())
				generatedLines.Add(Format(L"\t%", generatedBodyContents[i]));
			else if (i > 0)
				generatedLines.Add(Format(L"\t%\\", generatedBodyContents[i]));
			else
				generatedLines.Add(Format(L"%\\", generatedBodyContents[i]));
		}
	}

	return generatedLines;
}

Array<String> CogListenerType::GenerateSourceFileContents(const DocumentTemplates& aTemplates) const
{
	Array<String> sourceOutput = Base::GenerateSourceFileContents(aTemplates);

	sourceOutput.Add(Format(L"const TypeData& %::GetType() const", GetTypeName()));
	sourceOutput.Add(String(L"{"));
	sourceOutput.Add(String(L"\tstatic const TypeData& data = gTypeList.GetTypeData(GetStaticTypeName(), false);"));
	sourceOutput.Add(String(L"\treturn data;"));
	sourceOutput.Add(String(L"}"));

	return sourceOutput;
}
