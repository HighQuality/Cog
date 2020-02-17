#include "CogBuildPch.h"
#include "HeaderParser.h"
#include <Filesystem\File.h>
#include <CogBuildUtilities.h>
#include "CogClass.h"

HeaderParser::HeaderParser(const File* aMainFile, String aHeaderIncludePath)
	: myGeneratedCode(String(aMainFile->GetAbsolutePath()), aMainFile->GetFilenameWithoutExtension(), Move(aHeaderIncludePath))
{
	myFile = aMainFile;
	myFileContents = myFile->ReadString();
	myGeneratedHeaderFileName = Format(L"%.generated.h", aMainFile->GetFilenameWithoutExtension());
	myWordReader = GroupingWordReader(myFileContents.View());
}

void HeaderParser::Parse()
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
					return;
				}

				ParseCogType();
			}
			else if (currentWord == L"#")
			{
				if (!MoveNextExpectWord())
					return;

				currentWord = myWordReader.GetCurrentWordOrGroup();

				if (TryConsume(L"include"))
				{
					if (hasGeneratedHeaderInclude)
					{
						ReportError(L"The last include must be the generated header file: #include \"%\"", myGeneratedHeaderFileName.View());
						return;
					}

					const bool isOpenedByAngleBracket = IsAtGroup(L"<");
					const bool isOpenedByQuotationMarks = IsAtGroup(L"\"");

					if (isOpenedByAngleBracket || isOpenedByQuotationMarks)
					{
						StringView includePath = myWordReader.GetCurrentGroup();

						if (isOpenedByQuotationMarks && includePath == myGeneratedHeaderFileName.View())
						{
							hasGeneratedHeaderInclude = true;

							myGeneratedCode.SetHasGeneratedCode(true);
						}
					}
				}
			}
		}
	}
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
	else if (classType == L"struct")
	{
		TODO;
	}
	else if (classType == L"enum")
	{
		TODO;
	}
	else
	{
		ReportError(L"Expected \"struct\", \"class\" or \"enum\", got %", classType);
		return;
	}
}

void HeaderParser::ParseCogTypeClass(GroupingWordReader& aParameterReader)
{
	if (!MoveNextExpectWord())
		return;

	const i32 declarationLine = myWordReader.CalculateAndGetCurrentLineIndex();

	const StringView className = myWordReader.GetCurrentWordOrGroup();
	StringView baseClass;

	if (!MoveNextExpectWord())
		return;

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

	if (baseClass == L"IObject")
		baseClass = StringView();

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

	CogClass& cogClass = myGeneratedCode.AddCogClass(String(className), String(baseClass), declarationLine, generatedBodyLineIndex);

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
		else if (currentParameter == L"Replicated") /* Should be inherited by subclasses. */
		{
			TODO;
		}
		else if (currentParameter == L"SetDebugFlag")
		{
			cogClass.SetDebugFlag(true);
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
	if (!aBodyReader.Next() || !aBodyReader.IsAtGroup() || aBodyReader.GetOpeningSequence() != L"(")
	{
		ReportErrorWithInnerReader(aBodyReader, L"Expected '('");
		return false;
	}

	GroupingWordReader parameterReader(aBodyReader.GetCurrentGroup());
	parameterReader.SetLineOffset(aBodyReader.GetCurrentGroupFirstContentLineIndex());

	if (!parameterReader.Next())
	{
		ReportErrorWithInnerReader(parameterReader, L"Expected COGPROPERTY declaration");
		return false;
	}

	CogProperty newProperty;

	for (;;)
	{
		while (parameterReader.GetCurrentWord() == L"const" ||
			parameterReader.GetCurrentWord() == L"volatile")
		{
			newProperty.propertyType.Append(parameterReader.GetCurrentWord());
			newProperty.propertyType.Add(L' ');

			if (!parameterReader.Next())
			{
				ReportErrorWithInnerReader(parameterReader, L"Unexpected end of COGPROPERTY");
				return false;
			}
		}

		newProperty.propertyType.Append(parameterReader.GetCurrentWordOrGroup());

		if (!parameterReader.Next())
		{
			ReportErrorWithInnerReader(parameterReader, L"Unexpected end of COGPROPERTY");
			return false;
		}

		if (parameterReader.IsAtGroup())
		{
			newProperty.propertyType.Append(parameterReader.GetOpeningSequence());
			newProperty.propertyType.Append(parameterReader.GetCurrentWordOrGroup());
			newProperty.propertyType.Append(parameterReader.GetClosingSequence());

			if (!parameterReader.Next())
			{
				ReportErrorWithInnerReader(aBodyReader, L"Unexpected end of COGPROPERTY");
				return false;
			}
		}

		while (parameterReader.GetCurrentWord() == L"*" ||
			parameterReader.GetCurrentWord() == L"&" ||
			parameterReader.GetCurrentWord() == L"const")
		{
			newProperty.propertyType.Append(parameterReader.GetCurrentWord());
			newProperty.propertyType.Add(L' ');

			if (!parameterReader.Next())
			{
				ReportErrorWithInnerReader(parameterReader, L"Unexpected end of COGPROPERTY");
				return false;
			}
		}

		if (parameterReader.IsAtGroup())
		{
			ReportErrorWithInnerReader(parameterReader, L"Invalid C++ type in COGPROPERTY");
			return false;
		}

		if (parameterReader.GetCurrentWordOrGroup() == L"::")
		{
			newProperty.propertyType.Append(L"::");
			
			if (!parameterReader.Next())
			{
				ReportErrorWithInnerReader(parameterReader, L"Invalid C++ type in COGPROPERTY");
				return false;
			}

			continue;
		}
		
		break;
	}
	
	newProperty.propertyName = String(parameterReader.GetCurrentWordOrGroup());

	if (parameterReader.Next())
	{
		bool reachedEnd = false;

		if (parameterReader.GetCurrentWordOrGroup() == L"=")
		{
			newProperty.zeroMemory = false;

			if (parameterReader.Next())
			{
				for (;;)
				{
					if (parameterReader.IsAtGroup())
						newProperty.defaultValue.Append(parameterReader.GetOpeningSequence());

					newProperty.defaultValue.Append(parameterReader.GetCurrentWordOrGroup());

					if (parameterReader.IsAtGroup())
						newProperty.defaultValue.Append(parameterReader.GetClosingSequence());

					newProperty.defaultValue.Add(L' ');

					if (!parameterReader.Next())
					{
						reachedEnd = true;
						break;
					}

					if (parameterReader.GetCurrentWordOrGroup() == L",")
						break;
				}
			}
			else
			{
				ReportErrorWithInnerReader(aBodyReader, L"Invalid C++ type in COGPROPERTY");
				return false;
			}
		}

		if (!reachedEnd)
		{
			do
			{
				if (parameterReader.GetCurrentWordOrGroup() == L",")
				{
					if (!parameterReader.Next())
					{
						ReportErrorWithInnerReader(aBodyReader, L"Unexpected end of COGPROPERTY parameter list", parameterReader.GetCurrentWordOrGroup());
						return false;
					}

					const StringView parameter = parameterReader.GetCurrentWordOrGroup();

					if (parameter == L"DirectAccess") /* Generates "TYPE& PROPERTY();" and "const TYPE& PROPERTY() const;" */
					{
						newProperty.directAccess = true;
					}
					else if (parameter == L"PublicRead") /* Exposes const TYPE& GetPROPERTY() to other classes, not affected by DirectAccess. Be aware of thread synchronization. */
					{
						newProperty.publicRead = true;
					}
					else if (parameter == L"Replicated") /* Replicates over the network, should require "Replicated" in COGTYPE() or base class, a parameter which should be inherited. Incompatible with DirectAccess because we want to be notified of changes to these variables. */
					{
						TODO;
					}
					else if (parameter == L"OnChanged") /* OnChanged = FunctionName, calls function when value is changed, incompatible with DirectAccess so it can't be written to without our knowledge. (Should this happen immediately or be scheduled? Scheduling would be consistent but would make it hard to debug. Anything writing to it would also only have handled synchronization of that property only.) */
					{
						TODO;
					}
					else if (parameter == L"Borrowable") /* Generates "Borrowed<TYPE> BorrowPROPERTY()". This moves the property out of the chunk into the Borrowed instance and on destruction calls "SetPROPERTY(Move(myValue));". This allows for nicer access to variables that do not support DirectAccess due to change notifies. Accessing the underlying property during borrows is UB, could potentially be checked for in debug builds though. */
					{
						TODO;
					}
					else if (parameter == L"PerType") /* This variable is not stored per instance, but instead is shared between instances. Subclasses may override the default through a TBD syntax. */
					{
						TODO;
					}
					else if (parameter == L"ReadOnly") /* The value may never differ from the default, implies PerType. */
					{
						TODO;
					}
					else if (parameter == L"DoubleBuffered") /* Every frame, save a copy of this value so the old value safely can be accessed from other threads. GetBufferedPROPERTY() (?) (publicly accessible?) */
					{
						TODO;
					}
					else if (parameter == L"Config") /* Default value read from config file, default value from declaration is used if not specified in config file. Figure out priority system and syntax for these files. Subclasses that override the default value will override the config. If the config wants priority it should change the subclass' default for this property. */
					{
						TODO;
					}
					else if (parameter == L"Uninitialized") /* The memory occupied by this property should not be zeroed before a new instance gets access to it. We might want to be implicit if a default value is specified. */
					{
						newProperty.zeroMemory = false;
					}
					else
					{
						ReportErrorWithInnerReader(aBodyReader, L"Unknown COGPROPERTY parameter %", parameterReader.GetCurrentWordOrGroup());
						return false;
					}
				}
				else
				{
					ReportErrorWithInnerReader(aBodyReader, L"Expected ',' or end of COGPROPERTY declaration, instead got ", parameterReader.GetCurrentWordOrGroup());
					return false;
				}
			} while (parameterReader.Next());
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
	
	aClass.RegisterCogProperty(Move(newProperty));
	
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

bool HeaderParser::IsAtGroup(const StringView aOpener)
{
	if (myWordReader.IsAtGroup() && myWordReader.GetOpeningSequence() == aOpener)
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

	if (myWordReader.GetOpeningSequence() != L"(")
	{
		ReportError(L"Expected '(', got '%'", myWordReader.GetOpeningSequence());
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

	if (myWordReader.GetOpeningSequence() != L"{")
	{
		ReportError(L"Expected '{', got '%'", myWordReader.GetOpeningSequence());
		return false;
	}

	return true;
}
