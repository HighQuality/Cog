#pragma once
#include "CogType.h"
#include "GeneratedFunction.h"
#include "CogProperty.h"

struct ClassPropertyInitializerData;

class CogClass : public CogType
{
public:
	using Base = CogType;

	CogClass(String aAbsoluteFilePath, i32 aDeclarationLine, String aClassName, String aBaseClassName, i32 aGeneratedBodyLineIndex);

	Array<String> GenerateGeneratedBodyContents(StringView aGeneratedHeaderIdentifier) const;
	Array<String> GenerateCogTypeChunkHeaderContents() const;
	Array<String> GenerateCogTypeChunkSourceContents() const;

	Array<String> GenerateHeaderFileContents(const DocumentTemplates& aTemplates, StringView aGeneratedHeaderIdentifier) const override;
	Array<String> GenerateSourceFileContents(const DocumentTemplates& aTemplates) const override;

	void SetSpecializesBaseClass(const bool aSpecializesBaseClass) { mySpecializesBaseClass = aSpecializesBaseClass; }
	bool SpecializesBaseClass() const { return mySpecializesBaseClass; }

	void SetDebugFlag(const bool aDebugFlag) { myDebugFlag = aDebugFlag; }

	void SetIsFinal(bool aIsFinal);

	void RegisterCogProperty(CogProperty aProperty);

	void PostResolveDependencies() override;
	
protected:
	FORCEINLINE const CogClass* GetBaseType() const { return static_cast<const CogClass*>(Base::GetBaseType()); }
	const CogClass* GetRootClass() const;

	void GatherPropertyInitializers(Map<StringView, ClassPropertyInitializerData>& aPropertyInitializers) const;

private:
	Array<String> GenerateSingletonInitialization() const;

	Array<GeneratedFunction> myGeneratedFunctions;
	Array<CogProperty> myProperties;
	
	String myChunkTypeName;
	i32 myGeneratedBodyLineIndex = -1;
	bool mySpecializesBaseClass = false;
	bool myIsFinal = false;
	bool myDebugFlag = false;
	bool myIsSingleton = false;
};

struct ClassPropertyInitializerData
{
	StringView constructionArguments;
	StringView defaultAssignment;
	StringView propertyType;
	bool zeroMemory;
};
