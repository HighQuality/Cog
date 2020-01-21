#include "CogBuildPch.h"
#include "CogClass.h"
#include "String/StringTemplate.h"
#include "DocumentTemplates.h"

CogClass::CogClass(String aClassName, String aBaseClassName, i32 aGeneratedBodyLineIndex)
	: Base(Move(aClassName), Move(aBaseClassName))
{
	myGeneratedBodyLineIndex = aGeneratedBodyLineIndex;
	myChunkTypeName = Format(L"%CogTypeChunk", GetTypeName());
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

	generatedLines.Add(String(L"private:"));

	generatedLines.Add(Format(L"FORCEINLINE const %CogTypeChunk& GetChunk() const { return static_cast<const %CogTypeChunk&>(*myChunk); }", GetTypeName(), GetTypeName()));
	generatedLines.Add(Format(L"FORCEINLINE %CogTypeChunk& GetChunk() { return static_cast<%CogTypeChunk&>(*myChunk); }", GetTypeName(), GetTypeName()));

	// Reset the default visibility of class
	generatedLines.Add(String(L"private:"));

	return generatedLines;
}

Array<String> CogClass::GenerateCogTypeChunkHeaderContents() const
{
	Array<String> generatedLines;

	const String baseChunkName = Format(L"%CogTypeChunk", HasBaseType() ? GetBaseTypeName() : L"");
	const StringView finalSpecifier = myIsFinal ? L"final " : L"";

	generatedLines.Add(Format(L"class % %: public %", myChunkTypeName, finalSpecifier, baseChunkName));
	generatedLines.Add(String(L"{"));
	generatedLines.Add(String(L"public:"));
	generatedLines.Add(Format(L"\tusing Base = %;", baseChunkName));

	generatedLines.Add(Format(L"\tUniquePtr<Object> CreateDefaultObject() const override;"));

	generatedLines.Add(String(L"};"));
	
	return generatedLines;
}

Array<String> CogClass::GenerateCogTypeChunkSourceContents() const
{
	Array<String> generatedLines;

	generatedLines.Add(Format(L"UniquePtr<Object> %::CreateDefaultObject() const", myChunkTypeName));
	generatedLines.Add(String(L"{"));
	generatedLines.Add(Format(L"\treturn MakeUnique<%>();", GetTypeName()));
	generatedLines.Add(String(L"}"));

	return generatedLines;
}

String CogClass::GenerateHeaderFileContents(const DocumentTemplates& aTemplates, const StringView aGeneratedHeaderIdentifier) const
{
	String headerOutput = Base::GenerateHeaderFileContents(aTemplates, aGeneratedHeaderIdentifier);

	{
		const Array<String> generatedBodyContents = GenerateGeneratedBodyContents(aGeneratedHeaderIdentifier);

		for (i32 i = 0; i < generatedBodyContents.GetLength(); ++i)
		{
			if (i > 0)
				headerOutput.Append(L" \\\n\t");

			headerOutput.Append(generatedBodyContents[i].View());
		}

		headerOutput.Add(L'\n');
	}

	
	{
		const Array<String> chunkContents = GenerateCogTypeChunkHeaderContents();

		for (i32 i = 0; i < chunkContents.GetLength(); ++i)
		{
			if (i > 0)
				headerOutput.Add(L'\n');

			headerOutput.Append(chunkContents[i].View());
		}
	}

	return headerOutput;
}

String CogClass::GenerateSourceFileContents(const DocumentTemplates& aTemplates) const
{
	String sourceOutput = Base::GenerateSourceFileContents(aTemplates);

	{
		const Array<String> chunkContents = GenerateCogTypeChunkSourceContents();

		for (i32 i = 0; i < chunkContents.GetLength(); ++i)
		{
			if (i > 0)
				sourceOutput.Add(L'\n');

			sourceOutput.Append(chunkContents[i].View());
		}
	}
	
	return sourceOutput;
}

void CogClass::SetIsFinal(const bool aIsFinal)
{
	myIsFinal = aIsFinal;
}
