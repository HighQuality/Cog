#include "CogBuildPch.h"
#include "HeaderParser.h"
#include <Filesystem\File.h>
#include <CogBuildUtilities.h>

HeaderParser::HeaderParser(const File* aMainFile)
	: myGeneratedCode(aMainFile->GetFilenameWithoutExtension())
{
	myFile = aMainFile;
	myFileContents = myFile->ReadString();
	myGeneratedHeaderFileName = Format(L"%.generated.h", aMainFile->GetFilenameWithoutExtension());
	myWordReader = GroupingWordReader(myFileContents.View());
}

GeneratedCode& HeaderParser::Parse()
{
	bool hasGeneratedHeaderInclude = false;

	while (myWordReader.Next())
	{
		if (!myWordReader.IsAtGroup())
		{
			StringView currentWord = myWordReader.GetCurrentWordOrGroup();

			if (currentWord == L"COGTYPE")
			{
				if (!hasGeneratedHeaderInclude)
				{
					ReportError(L"Missing #include \"%\"", myGeneratedHeaderFileName.View());
					return myGeneratedCode;
				}

				ParseCogType();
			}
			else if (currentWord == L"#")
			{
				if (!MoveNextExpectWord())
					return myGeneratedCode;

				currentWord = myWordReader.GetCurrentWordOrGroup();

				if (TryConsume(L"include"))
				{
					if (hasGeneratedHeaderInclude)
					{
						ReportError(L"The last include must be the generated header file: #include \"%\"", myGeneratedHeaderFileName.View());
						return myGeneratedCode;
					}

					const bool isOpenedByAngleBracket = IsAtGroup(L'<');
					const bool isOpenedByQuotationMarks = IsAtGroup(L'\"');

					if (isOpenedByAngleBracket || isOpenedByQuotationMarks)
					{
						StringView includePath = myWordReader.GetCurrentGroup();

						if (isOpenedByQuotationMarks && includePath == myGeneratedHeaderFileName.View())
						{
							hasGeneratedHeaderInclude = true;

							myGeneratedCode.SetShouldGenerateCode(true);
						}
					}
				}
			}
		}
	}

	return myGeneratedCode;
}

void HeaderParser::ParseCogType()
{
	if (!MoveNextExpectParenthesisGroup())
		return;

	// GroupingWordReader parameterReader(reader.GetCurrentGroup());
	// 
	// while (parameterReader.Next());

	if (!MoveNextExpectWord())
		return;

	const StringView classType = myWordReader.GetCurrentWordOrGroup();

	if (classType == L"class")
	{
		ParseCogTypeClass();
	}
	else if (classType != L"struct")
	{
		TODO;
	}
	else if (classType == L"enum")
	{
		TODO;
	}
	else
	{
		ReportError(L"Expected \"struct\" or \"class\", got %", classType);
		return;
	}
}

void HeaderParser::ParseCogTypeClass()
{
	if (!MoveNextExpectWord())
		return;

	const StringView className = myWordReader.GetCurrentWordOrGroup();
	
	myWordReader.Next();

	TryConsume(L"final");
	
	if (!Expect(L":"))
		return;

	if (!Expect(L"public"))
		return;

	if (!myWordReader.IsAtWord())
	{
		ReportError(L"Expected word");
		return;
	}

	const StringView baseClass = myWordReader.GetCurrentWordOrGroup();

	if (!MoveNextExpectBracesGroup())
		return;

	i32 generatedBodyLineIndex = -1;

	GroupingWordReader bodyReader(myWordReader.GetCurrentWordOrGroup());
	bodyReader.SetLineOffset(myWordReader.GetCurrentGroupFirstContentLineIndex());

	if (bodyReader.Next() && bodyReader.IsAtWord() && bodyReader.GetCurrentWordOrGroup() == L"GENERATED_BODY")
	{
		generatedBodyLineIndex = bodyReader.CalculateAndGetCurrentLineIndex();

		if (!bodyReader.Next())
		{
			ReportErrorWithInnerReader(bodyReader, L"Unexpected end of group");
			return;
		}

		if (!bodyReader.IsAtWord() && bodyReader.GetCurrentWord() == L";")
		{
			ReportErrorWithInnerReader(bodyReader, L"Expected ';'");
			return;
		}
	}
	else
	{
		ReportErrorWithInnerReader(bodyReader, L"COGTYPE classes must begin with \"GENERATED_BODY;\", got \"%\"", bodyReader.GetCurrentWordOrGroup());
		return;
	}

	CogClass& cogClass = myGeneratedCode.AddCogClass(String(className), String(baseClass), generatedBodyLineIndex);

	// TODO: Remove
	cogClass;

	while (bodyReader.Next())
	{
		if (bodyReader.IsAtWord())
		{
			if (bodyReader.GetCurrentWordOrGroup() == L"GENERATED_BODY")
			{
				ReportErrorWithInnerReader(bodyReader, L"Found multiple generated bodies");
				return;
			}
		}
	}
}

void HeaderParser::ReportPreFormattedError(const StringView aMessage, GroupingWordReader* innerReader /* = nullptr*/)
{
	const i32 lineIndex = innerReader ? innerReader->CalculateAndGetCurrentLineIndex() : myWordReader.CalculateAndGetCurrentLineIndex();
	const i32 columnIndex = innerReader ? innerReader->CalculateAndGetCurrentColumnIndex() : myWordReader.CalculateAndGetCurrentColumnIndex();

	myErrors.Add(Format(L"%(%,%): error: %", myFile->GetAbsolutePath(), lineIndex + 1, columnIndex + 1, aMessage));
}

bool HeaderParser::At(const StringView aString) const
{
	if (myWordReader.IsAtGroup())
		return false;

	return myWordReader.GetCurrentWordOrGroup() == aString;
}

bool HeaderParser::Expect(const StringView aString)
{
	if (!At(aString))
	{
		ReportError(L"Expected %, got %", aString, myWordReader.GetCurrentWordOrGroup());
		return false;
	}

	myWordReader.Next();
	return true;
}

bool HeaderParser::TryConsume(const StringView aString)
{
	if (At(aString))
	{
		myWordReader.Next();
		return true;
	}

	return false;
}

bool HeaderParser::IsAtGroup(const Char aOpener)
{
	if (myWordReader.IsAtGroup() && myWordReader.GetOpeningCharacter() == aOpener)
	{
		return true;
	}

	return false;
}

bool HeaderParser::MoveNext()
{
	return myWordReader.Next();
}

bool HeaderParser::MoveNextExpectWord()
{
	if (!myWordReader.Next())
	{
		ReportError(L"Unexpected end of file");
		return false;
	}

	if (myWordReader.IsAtGroup())
	{
		ReportError(L"Expected word");
		return false;
	}

	return true;
}

bool HeaderParser::MoveNextExpectParenthesisGroup()
{
	if (!myWordReader.Next())
	{
		ReportError(L"Unexpected end of file");
		return false;
	}

	if (!myWordReader.IsAtGroup())
	{
		ReportError(L"Expected group");
		return false;
	}

	if (myWordReader.GetOpeningCharacter() != L'(')
	{
		ReportError(L"Expected '(', got '%'", myWordReader.GetOpeningCharacter());
		return false;
	}

	return true;
}

bool HeaderParser::MoveNextExpectBracesGroup()
{
	if (!myWordReader.Next())
	{
		ReportError(L"Unexpected end of file");
		return false;
	}

	if (!myWordReader.IsAtGroup())
	{
		ReportError(L"Expected group");
		return false;
	}

	if (myWordReader.GetOpeningCharacter() != L'{')
	{
		ReportError(L"Expected '{', got '%'", myWordReader.GetOpeningCharacter());
		return false;
	}

	return true;
}
