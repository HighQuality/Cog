#pragma once
#include "CogType.h"
#include "GeneratedFunction.h"

class GroupingWordReader;
class CogClass;
struct DocumentTemplates;

class GeneratedCode
{
public:
	GeneratedCode(StringView aMainFileName, String aMainHeaderIncludePath);

	void WriteFiles(const DocumentTemplates& aTemplates, StringView aProjectName, StringView aOutputDirectory);

	CogClass& AddCogClass(String aTypeName, String aBaseTypeName, i32 aGeneratedBodyLineIndex);

	void SetShouldGenerateCode(const bool aShouldGenerateCode) { myShouldGenerateCode = true; }
	bool ShouldGenerateCode() const { return myShouldGenerateCode; }

	StringView GetHeaderFileName() const { return myGeneratedHeaderFileName; }
	StringView GetSourceFileName() const { return myGeneratedSourceFileName; }

	ArrayView<CogClass*> GetCogClasses() const { return myCogClasses; }

private:
	void GenerateHeaderFile(const DocumentTemplates& aTemplates, StringView aHeaderFilePath);
	void GenerateSourceFile(const DocumentTemplates& aTemplates, StringView aProjectName, StringView aSourceFilePath);
	
	Map<String, u8> mySourceFileIncludes;
	Array<GeneratedFunction> myGlobalFunctions;
	
	Array<UniquePtr<CogType>> myDeclaredCogTypes;
	Array<CogClass*> myCogClasses;

	String myMainHeaderIncludePath;
	String myMainHeaderFileName;
	String myGeneratedHeaderFileName;
	String myGeneratedSourceFileName;
	String myGeneratedHeaderIdentifier;
	bool myShouldGenerateCode = false;
};
