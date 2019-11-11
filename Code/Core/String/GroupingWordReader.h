#pragma once
#include "WordReader.h"

class GroupingWordReader : private WordReader
{
public:
	using Base = WordReader;

	GroupingWordReader() = default;
	explicit GroupingWordReader(StringView aContent);

	bool Next();
	
	StringView GetCurrentWordOrGroup() const { return myCurrentContent; }
	StringView GetCurrentWord() const { CHECK(!IsAtGroup()); return myCurrentContent; }
	StringView GetCurrentGroup() const { CHECK(IsAtGroup()); return myCurrentContent; }

	bool IsAtGroup() const { return myIsAtGroup; }
	bool IsAtWord() const { return !myIsAtGroup; }

	Char GetOpeningCharacter() const { CHECK(IsAtGroup()); return myCurrentOpeningCharacter; }
	Char GetClosingCharacter() const { CHECK(IsAtGroup()); return myCurrentClosingCharacter; }

	/** Current refers to the word that was previously returned by NextWord */
	i32 CalculateAndGetCurrentLineIndex() { return Base::CalculateAndGetCurrentLineIndex() + myLineOffset; }
	/** Current refers to the word that was previously returned by NextWord */
	i32 CalculateAndGetCurrentColumnIndex() { return Base::CalculateAndGetCurrentColumnIndex(); }

	i32 GetCurrentGroupFirstContentLineIndex() const { CHECK(IsAtGroup()); return myCurrentGroupFirstContentLineIndex; }

	void SetLineOffset(const i32 aLineOffset) { myLineOffset = aLineOffset; }

private:
	static Char GetCorrespondingEndCharacter(Char aCharacter);

	StringView myCurrentContent;
	i32 myLineOffset = 0;
	i32 myCurrentGroupFirstContentLineIndex = 0;
	Char myCurrentOpeningCharacter = L'\0';
	Char myCurrentClosingCharacter = L'\0';
	Char myStopAtCharacter = L'\0';
	bool myIsAtGroup = false;
};
