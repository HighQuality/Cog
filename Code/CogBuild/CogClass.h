#pragma once
#include "CogType.h"
#include "GeneratedFunction.h"
#include "CogProperty.h"

class CogClass : public CogType
{
public:
	using Base = CogType;

	CogClass(String aClassName, String aBaseClassName, i32 aGeneratedBodyLineIndex);

	Array<String> GenerateGeneratedBodyContents(StringView aGeneratedHeaderIdentifier) const;
	Array<String> GenerateCogTypeChunkHeaderContents() const;
	Array<String> GenerateCogTypeChunkSourceContents() const;

	String GenerateHeaderFileContents(const DocumentTemplates& aTemplates, StringView aGeneratedHeaderIdentifier) const override;
	String GenerateSourceFileContents(const DocumentTemplates& aTemplates) const override;

	void SetSpecializesBaseClass(const bool aSpecializesBaseClass) { mySpecializesBaseClass = aSpecializesBaseClass; }
	bool SpecializesBaseClass() const { return mySpecializesBaseClass; }

	void SetIsFinal(bool aIsFinal);

	void RegisterCogProperty(CogProperty aProperty);
	
private:
	Array<GeneratedFunction> myGeneratedFunctions;
	Array<CogProperty> myProperties;
	
	String myChunkTypeName;
	i32 myGeneratedBodyLineIndex = -1;
	bool mySpecializesBaseClass = false;
	bool myIsFinal = false;
};

