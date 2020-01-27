#pragma once
#include "CogType.h"
#include "GeneratedFunction.h"

class GroupingWordReader;
class CogClass;
struct DocumentTemplates;

class GeneratedCode
{
public:
	GeneratedCode(String aMainHeaderFileAbsolutePath, StringView aMainFileName, String aMainHeaderIncludePath);

	void WriteFiles(const DocumentTemplates& aTemplates, StringView aProjectName, StringView aOutputDirectory) const;

	CogClass& AddCogClass(String aTypeName, String aBaseTypeName, i32 aDeclarationLine, i32 aGeneratedBodyLineIndex);

	void SetHasGeneratedCode(const bool aHasGeneratedCode) { myHasGeneratedCode = true; }
	bool HasGeneratedCode() const { return myHasGeneratedCode; }

	StringView GetHeaderFileName() const { return myGeneratedHeaderFileName; }
	StringView GetSourceFileName() const { return myGeneratedSourceFileName; }
	StringView GetMainHeaderIncludePath() const { return myMainHeaderIncludePath; }

	ArrayView<UniquePtr<CogType>> GetCogTypes() const { return myDeclaredCogTypes; }
	ArrayView<CogClass*> GetCogClasses() const { return myCogClasses; }

private:
	void GenerateHeaderFile(const DocumentTemplates& aTemplates, StringView aHeaderFilePath) const;
	void GenerateSourceFile(const DocumentTemplates& aTemplates, StringView aProjectName, StringView aSourceFilePath) const;
	
	Map<String, u8> mySourceFileIncludes;
	Array<GeneratedFunction> myGlobalFunctions;
	
	Array<UniquePtr<CogType>> myDeclaredCogTypes;
	Array<CogClass*> myCogClasses;

	String myMainHeaderFileAbsolutePath;
	String myMainHeaderIncludePath;
	String myMainHeaderFileName;
	String myGeneratedHeaderFileName;
	String myGeneratedSourceFileName;
	String myGeneratedHeaderIdentifier;
	bool myHasGeneratedCode = false;
};
