#include "CogBuildPch.h"
#include "CogClass.h"

CogClass::CogClass(String aClassName, i32 aGeneratedBodyLineIndex)
	: Base(Move(aClassName))
{
	myGeneratedBodyLineIndex = aGeneratedBodyLineIndex;
}

Array<String> CogClass::GenerateGeneratedBodyContents() const
{
	Array<String> generatedLines;

	generatedLines.Add(Format(L"#define GENERATED_BODY", GENERATED_HEADER_IDENTIFIER, GENERATED_BODY_LINE));
	generatedLines.Add(Format(L"using Base = %;", myBaseClassName));
	generatedLines.Add(String(L"void GetBaseClasses(const FunctionView<void(const TypeID<Object>&)>& aFunction) const override"));
	generatedLines.Add(String(L"{ aFunction(TypeID<Object>::Resolve<Base>()); Base::GetBaseClasses(aFunction); }"));

	return generatedLines;
}

String CogClass::GenerateHeaderFileContents() const
{
	String headerOutput = Base::GenerateHeaderFileContents();
	
	Array<String> generatedBodyContents = GenerateGeneratedBodyContents();

	for (i32 i = 0; i < generatedBodyContents.GetLength(); ++i)
	{
		if (i > 0)
			headerOutput.Append(L" \\\n\t");

		headerOutput.Append(generatedBodyContents[i].View());
	}

	return headerOutput;
}

String CogClass::GenerateSourceFileContents() const
{
	String sourceOutput = Base::GenerateHeaderFileContents();

	return sourceOutput;
}
