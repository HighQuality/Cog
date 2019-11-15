#include "CogBuildPch.h"
#include "GeneratedCode.h"
#include "CogClass.h"
#include "CogBuildUtilities.h"
#include "DocumentTemplates.h"
#include <String/StringTemplate.h>

GeneratedCode::GeneratedCode(const StringView aMainFileName)
{
	myGeneratedHeaderFileName = Format(L"%.generated.h", aMainFileName);
	myGeneratedSourceFileName = Format(L"%.generated.cpp", aMainFileName);

	myGeneratedHeaderIdentifier = Format(L"_%_H_", aMainFileName.ToUpper());
}

void GeneratedCode::WriteFiles(const DocumentTemplates& aTemplates, const StringView aProjectName, const StringView aOutputDirectory)
{
	CreateDirectoryW(aOutputDirectory.GetData(), nullptr);

	String headerFilePath(aOutputDirectory);
	headerFilePath.Add(L'\\');
	headerFilePath.Append(myGeneratedHeaderFileName);

	GenerateHeaderFile(aTemplates, headerFilePath);

	String sourceFilePath(aOutputDirectory);
	sourceFilePath.Add(L'\\');
	sourceFilePath.Append(myGeneratedSourceFileName);
	
	GenerateSourceFile(aTemplates, aProjectName, sourceFilePath);
}

CogClass& GeneratedCode::AddCogClass(String aTypeName, String aBaseTypeName, const i32 aGeneratedBodyLineIndex)
{
	CogClass& type = *new CogClass(Move(aTypeName), Move(aBaseTypeName), aGeneratedBodyLineIndex);
	// UniquePtr takes ownership of CogClass allocation
	myDeclaredCogTypes.Add(UniquePtr<CogType>(&type));
	myCogClasses.Add(&type);
	return type;
}

void GeneratedCode::GenerateHeaderFile(const DocumentTemplates& aTemplates, StringView aHeaderFilePath)
{
	StringTemplate document(String(aTemplates.generatedHeaderTemplate));

	document.AddParameter(String(L"GENERATED_HEADER_IDENTIFIER"), String(myGeneratedHeaderIdentifier));

	{
		String extraContents;

		for (const auto& cogType : myDeclaredCogTypes)
		{
			String generatedHeaderCode = cogType->GenerateHeaderFileContents(aTemplates, myGeneratedHeaderIdentifier);

			if (generatedHeaderCode.GetLength() > 0)
			{
				extraContents.Append(Format(L"// Generated code for type %\n", cogType->GetTypeName()).View());
				extraContents.Append(generatedHeaderCode.View());
				extraContents.Append(Format(L"\n// End generated code for type %\n", cogType->GetTypeName()).View());
			}
		}

		document.AddParameter(String(L"ExtraHeaderContent"), Move(extraContents));
	}

	const String headerFileContents = document.Evaluate();

	WriteToFileIfChanged(aHeaderFilePath, headerFileContents.View());
}

void GeneratedCode::GenerateSourceFile(const DocumentTemplates& aTemplates, const StringView aProjectName, const StringView aSourceFilePath)
{
	StringTemplate document(String(aTemplates.generatedSourceTemplate));

	document.AddParameter(String(L"HeaderFile"), String(myGeneratedHeaderFileName));
	document.AddParameter(String(L"PchFileName"), Format(L"%Pch.h", aProjectName));

	{
		String extraContents;

		for (const auto& cogType : myDeclaredCogTypes)
		{
			String generatedHeaderCode = cogType->GenerateSourceFileContents(aTemplates);

			if (generatedHeaderCode.GetLength() > 0)
			{
				extraContents.Append(Format(L"// Generated code for type %\n", cogType->GetTypeName()).View());
				extraContents.Append(generatedHeaderCode.View());
				extraContents.Append(Format(L"\n// End generated code for type %\n", cogType->GetTypeName()).View());
			}
		}

		document.AddParameter(String(L"ExtraSourceContent"), Move(extraContents));
	}

	const String sourceFileContents = document.Evaluate();

	WriteToFileIfChanged(aSourceFilePath, sourceFileContents.View());
}
