#pragma once
#include "WordReader.h"

enum class GroupingWordReaderGroup : u8
{
	None = 0,
	CurlyBraces = 1 << 0,
	Parenthesis = 1 << 1,
	SquareBracket = 1 << 2,
	AngleBracket = 1 << 3,

	CppSingleLineComment = 1 << 4,
	CppMultiLineComment = 1 << 5,
	CppLiteralStringWithEscapeCharacters = 1 << 6,

	// Since we can never change this without breaking old code it's better to have all categories opt-in
	Default = MaxOf<u8>
};

/**
 * When parsing strings where different characters have different meanings in different contexts this might not be what you want.
 * For example if you're parsing C++ '<' only signifies the start of a group when parsing a type name, otherwise it's a standalone operator.
 * This causes stuff like this to never find the last curly brace because the '<' group never ended: "{ if (a < b) { print(L"a is less than b"); } }"
 * It is possible to fix this by traversing the parent GroupingWordReader hierarchy when coming across a lone end character.
 */
class GroupingWordReader : private WordReader
{
public:
	using Base = WordReader;

	GroupingWordReader() = default;
	explicit GroupingWordReader(StringView aContent);

	StringView NextWord();
	bool NextWord(StringView aView);
	bool NextGroup(StringView aOpener);
	bool NextGroup(GroupingWordReader& aReader, StringView aOpener);
	bool Next();
	
	void EnableGroup(GroupingWordReaderGroup aGroup);
	void DisableGroup(GroupingWordReaderGroup aGroup);
	
	void DisableAllGroups();

	FORCEINLINE bool IsGroupEnabled(const GroupingWordReaderGroup aGroup) const { return myEnabledGroups & static_cast<u8>(aGroup); }

	StringView GetCurrentWordOrGroup() const { return myCurrentContent; }
	StringView GetCurrentWord() const { CHECK(!IsAtGroup()); return myCurrentContent; }
	StringView GetCurrentGroup() const { CHECK(IsAtGroup()); return myCurrentContent; }

	FORCEINLINE bool IsAtGroup() const { return myIsAtGroup; }
	bool IsAtGroup(StringView aOpeningSequence) const;

	FORCEINLINE bool IsAtWord() const { return !myIsAtGroup; }
	bool IsAtWord(StringView aWord) const;
	
	FORCEINLINE bool IsAtEnd() const { return myHasReachedEnd; }

	StringView GetOpeningSequence() const { CHECK(IsAtGroup()); return myCurrentOpeningSequence; }
	StringView GetClosingSequence() const { CHECK(IsAtGroup()); return myCurrentClosingSequence; }

	/** Current refers to the word that was previously returned by NextWord */
	i32 CalculateAndGetCurrentLineIndex() { return Base::CalculateAndGetCurrentLineIndex() + myLineOffset; }
	/** Current refers to the word that was previously returned by NextWord */
	i32 CalculateAndGetCurrentColumnIndex() { if (CalculateAndGetCurrentLineIndex() == myLineOffset) return Base::CalculateAndGetCurrentColumnIndex(); return Base::CalculateAndGetCurrentColumnIndex() + myColumnOffset; }

	i32 GetCurrentGroupFirstContentLineIndex() const { CHECK(IsAtGroup()); return myCurrentGroupFirstContentLineIndex; }

	void CopySettingsFrom(const GroupingWordReader& aOther);

private:
	bool GetGroupingPair(StringView aView, GroupingWordReaderGroup& aGroup, StringView& aOpeningSequence, StringView& aClosingSequence, bool &aIsOpening, bool &aIsClosing) const;
	bool AnyParentWantsToStopAtSequence(StringView aSequence, i32& aStopAtDepth, i32 aCurrentDepth = 0) const;
	static bool IsGroupExclusive(GroupingWordReaderGroup aGroup);

	StringView myCurrentContent;
	GroupingWordReader* myParentReader = nullptr;
	i32 myLineOffset = 0;
	i32 myColumnOffset = 0;
	i32 myCurrentGroupFirstContentLineIndex = 0;
	StringView myCurrentOpeningSequence;
	StringView myCurrentClosingSequence;
	u8 myEnabledGroups = static_cast<u8>(GroupingWordReaderGroup::Default);
	bool myIsAtGroup = false;
	bool myHasShortCircuited = false;
	bool myHasReachedEnd = false;
};
