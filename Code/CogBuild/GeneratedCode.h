#pragma once
#include "CogType.h"
#include "GeneratedFunction.h"
#include "PendingCogType.h"

class GroupingWordReader;
class CogClass;
struct DocumentTemplates;

class GeneratedCode
{
public:
	GeneratedCode(String aMainHeaderFileAbsolutePath, StringView aMainFileName, String aMainHeaderIncludePath);

	void WriteFiles(const DocumentTemplates& aTemplates, StringView aProjectName, StringView aOutputDirectory) const;

	void RegisterPendingType(PendingCogType aPendingType);
	ArrayView<PendingCogType> GetPendingTypes() const { return myPendingTypes; }

	CogClass& AddCogClass(String aTypeName, String aBaseTypeName, i32 aDeclarationLine, i32 aGeneratedBodyLineIndex);

	FORCEINLINE void SetHasGeneratedCode(const bool aHasGeneratedCode) { myHasGeneratedCode = true; }
	FORCEINLINE bool HasGeneratedCode() const { return myHasGeneratedCode; }

	FORCEINLINE StringView GetHeaderFileName() const { return myGeneratedHeaderFileName; }
	FORCEINLINE StringView GetSourceFileName() const { return myGeneratedSourceFileName; }
	FORCEINLINE StringView GetMainHeaderIncludePath() const { return myMainHeaderIncludePath; }

	FORCEINLINE ArrayView<UniquePtr<CogType>> GetCogTypes() const { return myDeclaredCogTypes; }
	FORCEINLINE ArrayView<CogClass*> GetCogClasses() const { return myCogClasses; }

private:
	void GenerateHeaderFile(const DocumentTemplates& aTemplates, StringView aHeaderFilePath) const;
	void GenerateSourceFile(const DocumentTemplates& aTemplates, StringView aProjectName, StringView aSourceFilePath) const;
	
	Map<String, u8> mySourceFileIncludes;
	Array<GeneratedFunction> myGlobalFunctions;

	Array<PendingCogType> myPendingTypes;
	
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
