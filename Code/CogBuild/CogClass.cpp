#include "CogBuildPch.h"
#include "CogClass.h"

CogClass::CogClass(String aClassName, String aBaseClassName, i32 aGeneratedBodyLineIndex)
	: Base(Move(aClassName), Move(aBaseClassName))
{
	myGeneratedBodyLineIndex = aGeneratedBodyLineIndex;
}

Array<String> CogClass::GenerateGeneratedBodyContents(const StringView aGeneratedHeaderIdentifier) const
{
	Array<String> generatedLines;

	generatedLines.Add(Format(L"#define GENERATED_BODY", aGeneratedHeaderIdentifier, myGeneratedBodyLineIndex + 1));
	generatedLines.Add(String(L"public:"));

	if (HasBaseType())
		generatedLines.Add(Format(L"using Base = %;", GetBaseTypeName()));
	else
		generatedLines.Add(Format(L"using Base = void;"));

	if (GetTypeName() != L"Object")
	{
		generatedLines.Add(String(L"void GetBaseClasses(const FunctionView<void(const TypeID<Object>&)>& aFunction) const override"));
		generatedLines.Add(String(L"{ aFunction(TypeID<Object>::Resolve<Base>()); Base::GetBaseClasses(aFunction); }"));
	}

	// Reset the default visibility of class
	generatedLines.Add(String(L"private:"));

	generatedLines.Add(Format(L"FORCEINLINE const %CogTypeChunk& GetChunk() const { return static_cast<const %CogTypeChunk&>(*myChunk); };", GetBaseTypeName()));
	generatedLines.Add(Format(L"FORCEINLINE %CogTypeChunk& GetChunk() { return static_cast<%CogTypeChunk&>(*myChunk); };", GetBaseTypeName()));

	// Reset the default visibility of class
	generatedLines.Add(String(L"private:"));

	return generatedLines;
}

String CogClass::GenerateHeaderFileContents(const DocumentTemplates& aTemplates, const StringView aGeneratedHeaderIdentifier) const
{
	String headerOutput = Base::GenerateHeaderFileContents(aTemplates, aGeneratedHeaderIdentifier);
	
	Array<String> generatedBodyContents = GenerateGeneratedBodyContents(aGeneratedHeaderIdentifier);

	for (i32 i = 0; i < generatedBodyContents.GetLength(); ++i)
	{
		if (i > 0)
			headerOutput.Append(L" \\\n\t");

		headerOutput.Append(generatedBodyContents[i].View());
	}

	return headerOutput;
}

String CogClass::GenerateSourceFileContents(const DocumentTemplates& aTemplates) const
{
	String sourceOutput = Base::GenerateSourceFileContents(aTemplates);

	return sourceOutput;
}
