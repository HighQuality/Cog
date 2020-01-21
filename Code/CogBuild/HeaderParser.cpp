#include "CogBuildPch.h"
#include "HeaderParser.h"
#include <Filesystem\File.h>
#include <CogBuildUtilities.h>
#include "CogClass.h"

HeaderParser::HeaderParser(const File* aMainFile, String aHeaderIncludePath)
	: myGeneratedCode(aMainFile->GetFilenameWithoutExtension(), Move(aHeaderIncludePath))
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

	GroupingWordReader parameterReader(myWordReader.GetCurrentGroup());

	if (!MoveNextExpectWord())
		return;

	const StringView classType = myWordReader.GetCurrentWordOrGroup();

	if (classType == L"class")
	{
		ParseCogTypeClass(parameterReader);
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

void HeaderParser::ParseCogTypeClass(GroupingWordReader& aParameterReader)
{
	if (!MoveNextExpectWord())
		return;

	const StringView className = myWordReader.GetCurrentWordOrGroup();
	StringView baseClass;

	if (className != L"Object")
	{
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

		baseClass = myWordReader.GetCurrentWordOrGroup();
	}

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

	while (aParameterReader.Next())
	{
		if (!aParameterReader.IsAtWord())
		{
			ReportErrorAtLine(L"Expected parameter", generatedBodyLineIndex);
			return;
		}

		const StringView currentParameter = aParameterReader.GetCurrentWordOrGroup();

		if (currentParameter == L"Specialization")
		{
			cogClass.SetSpecializesBaseClass(true);
		}
		else
		{
			ReportErrorAtLine(Format(L"Unknown parameter ", currentParameter).View(), generatedBodyLineIndex);
			return;
		}

		if (!aParameterReader.Next())
			break;

		if (aParameterReader.IsAtWord() && aParameterReader.GetCurrentWordOrGroup() != L",")
		{
			ReportErrorAtLine(L"Expected ','", generatedBodyLineIndex);
			return;
		}
	}

	while (bodyReader.Next())
	{
		if (bodyReader.IsAtWord())
		{
			const StringView currentSymbol = bodyReader.GetCurrentWordOrGroup();

			if (currentSymbol == L"GENERATED_BODY")
			{
				ReportErrorWithInnerReader(bodyReader, L"Found multiple generated bodies");
				return;
			}
			else if (currentSymbol == L"COGLISTENER")
			{
				if (!ParseCogListener(cogClass, bodyReader))
					return;
			}
			else if (currentSymbol == L"COGPROPERTY")
			{
				if (!ParseCogProperty(cogClass, bodyReader))
					return;
			}
		}
	}
}

bool HeaderParser::ParseCogListener(CogClass& aClass, GroupingWordReader& aBodyReader)
{
	if (!aBodyReader.Next() || !aBodyReader.IsAtWord() || aBodyReader.GetCurrentWordOrGroup() != L";")
	{
		ReportErrorWithInnerReader(aBodyReader, L"Expected ';'");
		return false;
	}

	if (!aBodyReader.Next())
	{
		ReportErrorWithInnerReader(aBodyReader, L"Unexpected end of body");
		return false;
	}

	const bool isVirtual = aBodyReader.GetCurrentWordOrGroup() == L"virtual";

	if (isVirtual)
	{
		if (!aBodyReader.Next())
		{
			ReportErrorWithInnerReader(aBodyReader, L"Unexpected end of body");
			return false;
		}
	}

	if (!aBodyReader.IsAtWord() || aBodyReader.GetCurrentWordOrGroup() != L"void")
	{
		ReportErrorWithInnerReader(aBodyReader, L"Expected 'void'");
		return false;
	}

	if (!aBodyReader.Next() || !aBodyReader.IsAtWord())
	{
		ReportErrorWithInnerReader(aBodyReader, L"Expected method name");
		return false;
	}

	const StringView methodName = aBodyReader.GetCurrentWordOrGroup();

	Println(L"COGLISTENER % registered for type % and is %", methodName, aClass.GetTypeName(), isVirtual ? L"virtual" : L"not virtual");

	return true;
}

bool HeaderParser::ParseCogProperty(CogClass& aClass, GroupingWordReader& aBodyReader)
{
	if (!aBodyReader.Next() || !aBodyReader.IsAtGroup() || aBodyReader.GetOpeningCharacter() != L'(')
	{
		ReportErrorWithInnerReader(aBodyReader, L"Expected '('");
		return false;
	}

	GroupingWordReader parameterReader(aBodyReader.GetCurrentGroup());
	parameterReader.SetLineOffset(aBodyReader.GetCurrentGroupFirstContentLineIndex());

	if (!parameterReader.Next())
	{
		ReportErrorWithInnerReader(aBodyReader, L"Expected COGPROPERTY declaration");
		return false;
	}

	String propertyType;

	while (parameterReader.GetCurrentWord() == L"const"/* ||
		parameterReader.GetCurrentWord() == L"volatile"*/)
	{
		propertyType.Append(parameterReader.GetCurrentWord());
		propertyType.Add(L' ');

		if (!parameterReader.Next())
		{
			ReportErrorWithInnerReader(aBodyReader, L"Unexpected end of COGPROPERTY");
			return false;
		}
	}

	propertyType.Append(parameterReader.GetCurrentWordOrGroup());

	if (parameterReader.IsAtGroup())
	{

	}

	if (parameterReader.Next())
	{
		while (parameterReader.GetCurrentWord() == L"*" ||
			parameterReader.GetCurrentWord() == L"&" ||
			parameterReader.GetCurrentWord() == L"const")
		{
			propertyType.Append(parameterReader.GetCurrentWord());
			propertyType.Add(L' ');

			if (!parameterReader.Next())
			{
				ReportErrorWithInnerReader(aBodyReader, L"Unexpected end of COGPROPERTY");
				return false;
			}
		}

		while (parameterReader.Next())
		{

		}
	}

	if (!aBodyReader.Next())
	{
		ReportErrorWithInnerReader(aBodyReader, L"Unexpected end of body");
		return false;
	}

	if (!aBodyReader.IsAtWord() || aBodyReader.GetCurrentWordOrGroup() != L";")
	{
		ReportErrorWithInnerReader(aBodyReader, L"Expected ';'");
		return false;
	}

	Println(L"COGPROPERTY '%' of type '%' registered in type %", propertyName, propertyType, aClass.GetTypeName());

	return true;
}

void HeaderParser::ReportErrorAtLine(const StringView aMessage, const  i32 aLineIndex)
{
	myErrors.Add(Format(L"%(%): error: %", myFile->GetAbsolutePath(), aLineIndex + 1, aMessage));
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
		if (myWordReader.IsAtWord())
			ReportError(L"Expected %, got %", aString, myWordReader.GetCurrentWordOrGroup());
		else
			ReportError(L"Expected %, got group");

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
