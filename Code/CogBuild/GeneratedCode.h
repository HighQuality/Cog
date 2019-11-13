#pragma once
#include "CogType.h"
#include "GeneratedFunction.h"

class GroupingWordReader;
class CogClass;
struct DocumentTemplates;

class GeneratedCode
{
public:
	GeneratedCode(StringView aMainFileName);

	void WriteFiles(const DocumentTemplates& aTemplates, const StringView aOutputDirectory);

	CogClass& AddCogClass(String aTypeName, i32 aGeneratedBodyLineIndex);

	void SetShouldGenerateCode(const bool aShouldGenerateCode) { myShouldGenerateCode = true; }
	bool ShouldGenerateCode() const { return myShouldGenerateCode; }

private:
	void GenerateHeaderFile(const DocumentTemplates& aTemplates, StringView aHeaderFilePath);
	void GenerateSourceFile(const DocumentTemplates& aTemplates, StringView aSourceFilePath);
	
	Map<String, u8> mySourceFileIncludes;
	Array<GeneratedFunction> myGlobalFunctions;
	Array<UniquePtr<CogType>> myDeclaredCogTypes;
	String myGeneratedHeaderFileName;
	String myGeneratedSourceFileName;
	bool myShouldGenerateCode = false;
};
