#include "CogBuildPch.h"
#include "GeneratedCode.h"
#include "CogClass.h"
#include "CogBuildUtilities.h"

GeneratedCode::GeneratedCode(const StringView aMainFileName)
{
	myGeneratedHeaderFileName = Format(L"%_generated.h", aMainFileName);
	myGeneratedSourceFileName = Format(L"%_generated.cpp", aMainFileName);
}

void GeneratedCode::WriteFiles(const DocumentTemplates& aTemplates, const StringView aOutputDirectory)
{
	CreateDirectoryW(aOutputDirectory.GetData(), nullptr);

	String headerFilePath(aOutputDirectory);
	headerFilePath.Append(myGeneratedHeaderFileName);

	GenerateHeaderFile(aTemplates, headerFilePath);

	String sourceFilePath(aOutputDirectory);
	sourceFilePath.Append(myGeneratedSourceFileName);
	
	GenerateSourceFile(aTemplates, sourceFilePath);
}

CogClass& GeneratedCode::AddCogClass(String aTypeName, const i32 aGeneratedBodyLineIndex)
{
	CogClass& type = *new CogClass(Move(aTypeName), aGeneratedBodyLineIndex);
	// UniquePtr takes ownership of CogClass allocation
	myDeclaredCogTypes.Add(UniquePtr<CogType>(&type));
	return type;
}

void GeneratedCode::GenerateHeaderFile(const DocumentTemplates& aTemplates, StringView aHeaderFilePath)
{
	String headerFileContents;
	
	for (const auto& cogType : myDeclaredCogTypes)
	{
		String generatedHeaderCode = cogType->GenerateHeaderFileContents();
		
		if (generatedHeaderCode.GetLength() > 0)
		{
			headerFileContents.Append(Format(L"// Generated code for type %\n", cogType->GetTypeName()).View());
			headerFileContents.Append(generatedHeaderCode.View());
			headerFileContents.Append(Format(L"\n// End generated code for type %\n", cogType->GetTypeName()).View());
		}
	}

	WriteToFileIfChanged(aHeaderFilePath, headerFileContents.View());
}

void GeneratedCode::GenerateSourceFile(const DocumentTemplates& aTemplates, StringView aSourceFilePath)
{
	String sourceFileContents;
	
	for (const auto& cogType : myDeclaredCogTypes)
	{
		String generatedSourceCode = cogType->GenerateSourceFileContents();

		if (generatedSourceCode.GetLength() > 0)
		{
			sourceFileContents.Append(Format(L"// Generated code for type %\n", cogType->GetTypeName()).View());
			sourceFileContents.Append(generatedSourceCode.View());
			sourceFileContents.Append(Format(L"\n// End generated code for type %\n", cogType->GetTypeName()).View());
		}
	}

	WriteToFileIfChanged(aSourceFilePath, sourceFileContents.View());
}
