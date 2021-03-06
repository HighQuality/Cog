#pragma once
#include <String/GroupingWordReader.h>
#include "GeneratedCode.h"

class File;
class GroupingWordReader;
class CogClass;

class HeaderParser
{
public:
	HeaderParser(const File* aMainFile, String aHeaderIncludePath);

	void Parse();

	FORCEINLINE bool HasErrors() const { return myErrors.GetLength() > 0; }
	FORCEINLINE ArrayView<String> GetErrors() { return myErrors; }

	FORCEINLINE bool HasGeneratedCode() const { return myGeneratedCode.HasGeneratedCode(); }

	FORCEINLINE const GeneratedCode& GetGeneratedCode() const { return myGeneratedCode; }

private:
	void ParseCogType();
	void ParseCogTypeClass(GroupingWordReader& aParameterReader);
	bool ParseCogListener(CogClass& aClass, GroupingWordReader& aBodyReader);
	bool ParseCogProperty(CogClass& aClass, GroupingWordReader& aBodyReader);
	void ReportErrorAtLine(StringView aMessage, i32 aLine);
	void ReportPreFormattedError(StringView aMessage, GroupingWordReader* innerReader = nullptr);

	bool Expect(StringView aString);

	template <typename ...TArgs>
	void ReportError(TArgs ...aArgs)
	{
		const String message = Format(Move(aArgs)...);
		ReportPreFormattedError(message.View());
	}

	template <typename ...TArgs>
	void ReportErrorWithInnerReader(GroupingWordReader& innerReader, TArgs ...aArgs)
	{
		const String message = Format(Move(aArgs)...);
		ReportPreFormattedError(message.View(), &innerReader);
	}
	
	const File* myFile = nullptr;
	Array<String> myErrors;
	String myFileContents;
	String myGeneratedHeaderFileName;
	GroupingWordReader myWordReader;
	GeneratedCode myGeneratedCode;
	bool myFileHasCogTypes = false;
};

