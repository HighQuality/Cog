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
				if (myWordReader.NextWord(L"include"))
				{
					if (hasGeneratedHeaderInclude)
					{
						ReportError(L"The last include must be the generated header file: #include \"%\"", myGeneratedHeaderFileName.View());
						return;
					}

					GroupingWordReader includeReader;
					if (myWordReader.NextGroup(includeReader, L"\""))
					{
						if (myWordReader.GetCurrentGroup() == myGeneratedHeaderFileName.View())
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
	if (!myWordReader.NextGroup(L"("))
	{
		ReportError(L"Expected '('");
		return;
	}

	const StringView parameterListContent = myWordReader.GetCurrentWordOrGroup();
	
	if (!myWordReader.NextWord())
	{
		ReportError(L"Expected word");
		return;
	}

	const StringView classType = myWordReader.GetCurrentWordOrGroup();

	if (!myWordReader.NextWord())
	{
		ReportError(L"Syntax error");
		return;
	}

	const i32 declarationLine = myWordReader.CalculateAndGetCurrentLineIndex();

	const StringView typeName = myWordReader.GetCurrentWordOrGroup();

	myWordReader.NextWord(L"final");

	if (!Expect(L":"))
		return;

	if (!Expect(L"public"))
		return;

	if (!myWordReader.NextWord())
	{
		ReportError(L"Expected word");
		return;
	}

	const StringView baseClass = myWordReader.GetCurrentWordOrGroup();

	if (!myWordReader.NextGroup(L"{"))
	{
		ReportError(L"Expected '{'");
		return;
	}

	const StringView bodyContent = myWordReader.GetCurrentWordOrGroup();

	PendingCogType pendingType;
	pendingType.type = String(classType);
	pendingType.typeName = String(typeName);
	pendingType.baseType = String(baseClass);
	pendingType.body = String(bodyContent);
	pendingType.parameterList = String(parameterListContent);
	myGeneratedCode.RegisterPendingType(Move(pendingType));
}

void HeaderParser::ParseCogTypeClass(GroupingWordReader& aParameterReader)
{
	i32 generatedBodyLineIndex = -1;

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
		else if (currentParameter == L"ManualSpecialization") /* Specialization that's only activate after calling ProgramContext::ActivateSpecialization<T>(). */
		{
			TODO;
		}
		else if (currentParameter == L"UnitTestSpecialization") /* ManualSpecialization that's automatically activated when running unit tests. */
		{
			TODO;
		}
		else if (currentParameter == L"Replicated") /* Should be inherited by subclasses. */
		{
			TODO;
		}
		else if (currentParameter == L"Version") /* Version = <integer>, used for identifying the version when serializing a something. When serializing, if highest bit is 1, go to next data type. */
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
	if (!aBodyReader.NextWord(L";"))
	{
		ReportErrorWithInnerReader(aBodyReader, L"Expected ';'");
		return false;
	}

	aBodyReader.NextWord(L"virtual");

	if (!aBodyReader.NextWord(L"void"))
	{
		ReportErrorWithInnerReader(aBodyReader, L"Expected 'void'");
		return false;
	}

	if (!aBodyReader.NextWord())
	{
		ReportErrorWithInnerReader(aBodyReader, L"Expected method name");
		return false;
	}

	CogListener listener;
	listener.methodName = String(aBodyReader.GetCurrentWordOrGroup());

	GroupingWordReader parameterReader;
	if (!aBodyReader.NextGroup(parameterReader, L"("))
	{
		ReportErrorWithInnerReader(aBodyReader, L"Expected '('");
		return false;
	}

	if (!parameterReader.NextWord(L"const"))
	{
		ReportErrorWithInnerReader(parameterReader, L"Expected 'const'");
		return false;
	}

	if (!parameterReader.NextWord())
	{
		ReportErrorWithInnerReader(parameterReader, L"Expected type name");
		return false;
	}

	listener.listenerType = parameterReader.GetCurrentWordOrGroup();

	if (!parameterReader.NextWord(L"&"))
	{
		ReportErrorWithInnerReader(parameterReader, L"Expected '&'");
		return false;
	}

	if (listener.listenerType.EndsWith(L"Event"))
	{
		aClass.RegisterEventListener(Move(listener));
	}
	else if (listener.listenerType.EndsWith(L"Impulse"))
	{
		aClass.RegisterImpulseListener(Move(listener));
	}
	else
	{
		ReportErrorWithInnerReader(parameterReader, L"Types listened on must end with either 'Event' or 'Impulse'");
	}

	return true;
}

bool HeaderParser::ParseCogProperty(CogClass& aClass, GroupingWordReader& aBodyReader)
{
	GroupingWordReader parameterReader;
	if (!aBodyReader.NextGroup(parameterReader, L"("))
	{
		ReportErrorWithInnerReader(aBodyReader, L"Expected '('");
		return false;
	}

	if (!parameterReader.Next())
	{
		ReportErrorWithInnerReader(parameterReader, L"Expected COGPROPERTY declaration");
		return false;
	}

	CogProperty newProperty;

	bool typeCanBeName = true;

	for (;;)
	{
		while (parameterReader.GetCurrentWord() == L"const" ||
			parameterReader.GetCurrentWord() == L"volatile")
		{
			typeCanBeName = false;

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

		if (parameterReader.IsAtGroup() && parameterReader.GetOpeningSequence() != L"(")
		{
			typeCanBeName = false;

			newProperty.propertyType.Append(parameterReader.GetOpeningSequence());
			newProperty.propertyType.Append(parameterReader.GetCurrentWordOrGroup());
			newProperty.propertyType.Append(parameterReader.GetClosingSequence());

			if (!parameterReader.Next())
			{
				ReportErrorWithInnerReader(aBodyReader, L"Unexpected end of COGPROPERTY");
				return false;
			}
		}

		while (parameterReader.GetCurrentWordOrGroup() == L"*" ||
			parameterReader.GetCurrentWordOrGroup() == L"&" ||
			parameterReader.GetCurrentWordOrGroup() == L"const")
		{
			typeCanBeName = false;

			newProperty.propertyType.Append(parameterReader.GetCurrentWord());
			newProperty.propertyType.Add(L' ');

			if (!parameterReader.Next())
			{
				ReportErrorWithInnerReader(parameterReader, L"Unexpected end of COGPROPERTY");
				return false;
			}
		}

		if (parameterReader.IsAtGroup())
			break;

		if (parameterReader.GetCurrentWordOrGroup() == L"::")
		{
			typeCanBeName = false;

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

	bool canContinue;

	if (parameterReader.GetCurrentWordOrGroup() != L"=" &&
		parameterReader.GetCurrentWordOrGroup() != L"," &&
		!(parameterReader.IsAtGroup() && parameterReader.GetOpeningSequence() == L"(") &&
		parameterReader.GetCurrentWordOrGroup() != L"")
	{
		newProperty.propertyName = String(parameterReader.GetCurrentWordOrGroup());
		canContinue = parameterReader.Next();
	}
	else if (typeCanBeName)
	{
		newProperty.propertyName = String(newProperty.propertyType);
		canContinue = true;
	}
	else
	{
		ReportErrorWithInnerReader(parameterReader, L"Missing COGPROPERTY name");
		return false;
	}

	if (canContinue)
	{
		bool reachedEnd = false;
		const bool isConstructorCall = parameterReader.IsAtGroup() && parameterReader.GetOpeningSequence() == L"(";

		if (parameterReader.GetCurrentWordOrGroup() == L"=" ||
			isConstructorCall)
		{
			newProperty.zeroMemory = !isConstructorCall;

			String initString;

			if (!isConstructorCall)
			{
				if (!parameterReader.Next())
				{
					ReportErrorWithInnerReader(aBodyReader, L"Invalid C++ type in COGPROPERTY");
					return false;
				}

				for (;;)
				{
					if (parameterReader.IsAtGroup())
						initString.Append(parameterReader.GetOpeningSequence());

					initString.Append(parameterReader.GetCurrentWordOrGroup());

					if (parameterReader.IsAtGroup())
						initString.Append(parameterReader.GetClosingSequence());

					if (!parameterReader.Next())
					{
						reachedEnd = true;
						break;
					}

					initString.Add(L' ');

					if (parameterReader.GetCurrentWordOrGroup() == L",")
						break;
				}
			}
			else
			{
				initString = parameterReader.GetCurrentWordOrGroup();

				if (!parameterReader.Next())
					reachedEnd = true;
			}

			if (isConstructorCall)
				newProperty.constructionArguments = Move(initString);
			else
				newProperty.defaultAssignment = Move(initString);
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
					else if (parameter == L"Config") /* Default value read from config file, default value from declaration is used if not specified in config file or command line. Figure out priority system and syntax for these files. Subclasses that override the default value will override the config. If the config wants priority it should change the subclass' default for this property. */
					{
						TODO;
					}
					else if (parameter == L"Uninitialized") /* The memory occupied by this property should not be zeroed before a new instance gets access to it. We might want to be implicit if a default value is specified. */
					{
						newProperty.zeroMemory = false;
					}
					else if (parameter == L"CommandLine") /* CommandLine = (Name = "commandline", Required/Optional, Help = ""). Has higer priority than Config. */
					{
						TODO;
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
	String message = Format(L"%(%): error: %", myFile->GetAbsolutePath(), aLineIndex + 1, aMessage);
	ENSURE_MSG(false, L"%", message);
	myErrors.Add(Move(message));
}

void HeaderParser::ReportPreFormattedError(const StringView aMessage, GroupingWordReader* innerReader /* = nullptr*/)
{
	const i32 lineIndex = innerReader ? innerReader->CalculateAndGetCurrentLineIndex() : myWordReader.CalculateAndGetCurrentLineIndex();
	const i32 columnIndex = innerReader ? innerReader->CalculateAndGetCurrentColumnIndex() : myWordReader.CalculateAndGetCurrentColumnIndex();

	String message = Format(L"%(%,%): error: %", myFile->GetAbsolutePath(), lineIndex + 1, columnIndex + 1, aMessage);
	ENSURE_MSG(false, L"%", message);
	myErrors.Add(Move(message));
}

bool HeaderParser::Expect(const StringView aString)
{
	if (!myWordReader.NextWord(aString))
	{
		if (myWordReader.IsAtWord())
			ReportError(L"Expected %, got %", aString, myWordReader.GetCurrentWordOrGroup());
		else
			ReportError(L"Expected %, got group starting with %", aString, myWordReader.GetOpeningSequence());

		return false;
	}

	return true;
}
