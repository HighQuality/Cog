#pragma once
#include <String/GroupingWordReader.h>
#include "GeneratedCode.h"

class File;

class HeaderParser
{
public:
	HeaderParser(const File* aMainFile);

	void Parse();
	GeneratedCode GenerateCode();

	bool HasErrors() const { return myErrors.GetLength() > 0; }
	ArrayView<String> GetErrors() { return myErrors; }

private:
	void ParseCogType();
	void ParseCogTypeClass();
	void ReportPreFormattedError(StringView aMessage, GroupingWordReader* innerReader = nullptr);

	bool At(StringView aString) const;
	bool Expect(StringView aString);
	bool TryConsume(StringView aString);
	bool IsAtGroup(Char aOpener);
	bool MoveNext();
	bool MoveNextExpectWord();
	bool MoveNextExpectParenthesisGroup();
	bool MoveNextExpectBracesGroup();

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
	bool myFileHasCogTypes = false;
};

