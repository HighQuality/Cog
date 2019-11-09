#pragma once
#include "WordReader.h"

class GroupingWordReader : private WordReader
{
public:
	using Base = WordReader;

	explicit GroupingWordReader(StringView aContent);

	bool Next();
	
	StringView GetCurrentWordOrGroup() { return myCurrentContent; }
	StringView GetCurrentWord() { CHECK(!IsAtGroup()); return myCurrentContent; }
	StringView GetCurrentGroup() { CHECK(IsAtGroup()); return myCurrentContent; }

	bool IsAtGroup() const { return myIsAtGroup; }

	Char GetOpeningCharacter() const { CHECK(IsAtGroup()); return myCurrentOpeningCharacter; }
	Char GetClosingCharacter() const { CHECK(IsAtGroup()); return myCurrentClosingCharacter; }

private:
	static Char GetCorrespondingEndCharacter(Char aCharacter);

	StringView myCurrentContent;
	Char myCurrentOpeningCharacter;
	Char myCurrentClosingCharacter;
	bool myIsAtGroup = false;
};

