#include "CorePch.h"
#include "GroupingWordReader.h"

GroupingWordReader::GroupingWordReader(const StringView aContent)
	: Base(aContent)
{
}

bool GroupingWordReader::Next()
{
	myIsAtGroup = false;
	
	const StringView current = NextWord();

	if (!current)
		return false;

	GroupingWordReaderGroup currentGroup = GroupingWordReaderGroup::None;

	for (i32 i = 0; i < current.GetLength(); ++i)
	{
		StringView openingSequence;
		StringView closingSequence;

		bool isOpening;
		bool isClosing;

		if (GetGroupingPair(current.ChopFromStart(i), currentGroup, openingSequence, closingSequence, isOpening, isClosing))
		{
			if (isClosing)
			{
				i32 stopAtDepth = 0;
				if (AnyParentWantsToStopAtSequence(closingSequence, stopAtDepth))
				{
					SetReadIndex(GetReadIndex() - current.GetLength() + i);

					if (i > 0)
					{
						myCurrentContent = current.SliceFromStart(i);
						return true;
					}

					GroupingWordReader* currentParent = myParentReader;
					for (i32 iShortCircuit = 0; iShortCircuit < stopAtDepth; ++iShortCircuit)
					{
						currentParent->myHasShortCircuited = true;
						currentParent = currentParent->myParentReader;
					}

					return false;
				}
			}

			if (isOpening)
			{
				if (i > 0)
				{
					SetReadIndex(GetReadIndex() - current.GetLength() + i);
					myCurrentContent = current.SliceFromStart(i);
					return true;
				}
					
				SetReadIndex(GetReadIndex() - current.GetLength() + i + openingSequence.GetLength());
				myIsAtGroup = true;
				myCurrentOpeningSequence = openingSequence;
				myCurrentClosingSequence = closingSequence;
				break;
			}
		}
	}
	
	if (!myIsAtGroup)
	{
		myCurrentContent = current;
		return true;
	}

	CHECK(currentGroup != GroupingWordReaderGroup::None);

	{
		const i32 readHeader = GetReadIndex();
		NextWord();
		myCurrentGroupFirstContentLineIndex = CalculateAndGetCurrentLineIndex();
		SetReadIndex(readHeader);
	}

	const bool exclusiveMode = IsGroupExclusive(currentGroup);

	const i32 innerReaderStart = GetReadIndex();
	const StringView innerReaderString = GetString().ChopFromStart(innerReaderStart);
	i32 innerReaderStop = -1;

	if (!exclusiveMode)
	{
		// Include nested groups inside this group, also fixes cases where end character inside of our own group ends the group prematurely
		GroupingWordReader innerReader(innerReaderString);
		innerReader.CopySettingsFrom(*this);
		innerReader.SetIgnoreNewlines(false);
		innerReader.myParentReader = this;

		while (innerReader.Next())
		{ }

		innerReaderStop = innerReaderStart + innerReader.GetReadIndex();
	}
	else
	{
		const i32 innerLength = innerReaderString.GetLength();
		for (i32 i = 0; i < innerLength; ++i)
		{
			if (innerReaderString.ChopFromStart(i).StartsWith(myCurrentClosingSequence))
			{
				innerReaderStop = innerReaderStart + i;
				break;
			}
		}

		if (innerReaderStop == -1)
		{
			myHasShortCircuited = true;
			innerReaderStop = innerReaderStart + innerLength;
		}
	}

	CHECK(innerReaderStop >= 0);

	if (!myHasShortCircuited)
		SetReadIndex(innerReaderStop + myCurrentClosingSequence.GetLength());
	else
		SetReadIndex(innerReaderStop);

	myCurrentContent = GetString().Slice(innerReaderStart, innerReaderStop - innerReaderStart);
	myCurrentContent.Trim();

	return !myHasShortCircuited;
}

void GroupingWordReader::EnableGroup(GroupingWordReaderGroup aGroup)
{
	myEnabledGroups |= static_cast<u8>(aGroup);
}

void GroupingWordReader::DisableGroup(GroupingWordReaderGroup aGroup)
{
	myEnabledGroups &= ~static_cast<u8>(aGroup);
}

void GroupingWordReader::DisableAllGroups()
{
	myEnabledGroups = 0;
}

void GroupingWordReader::CopySettingsFrom(const GroupingWordReader& aOther)
{
	myEnabledGroups = aOther.myEnabledGroups;
	SetIgnoreNewlines(aOther.IsIgnoringNewlines());
}

bool GroupingWordReader::GetGroupingPair(StringView aView, GroupingWordReaderGroup& aGroup, StringView& aOpeningSequence, StringView& aClosingSequence, bool &aIsOpening, bool& aIsClosing) const
{
	if (aView.GetLength() == 0)
		return false;

	switch (aView[0])
	{
	case L'{':
	case L'}':
		if (IsGroupEnabled(GroupingWordReaderGroup::CurlyBraces))
		{
			aGroup = GroupingWordReaderGroup::CurlyBraces;
			aOpeningSequence = L"{";
			aClosingSequence = L"}";
			aIsOpening = aView[0] == aOpeningSequence[0];
			aIsClosing = !aIsOpening;
			return true;
		}
		break;
	case L'(':
	case L')':
		if (IsGroupEnabled(GroupingWordReaderGroup::Parenthesis))
		{
			aGroup = GroupingWordReaderGroup::Parenthesis;
			aOpeningSequence = L"(";
			aClosingSequence = L")";
			aIsOpening = aView[0] == aOpeningSequence[0];
			aIsClosing = !aIsOpening;
			return true;
		}
		break;
	case L'[':
	case L']':
		if (IsGroupEnabled(GroupingWordReaderGroup::SquareBracket))
		{
			aGroup = GroupingWordReaderGroup::SquareBracket;
			aOpeningSequence = L"[";
			aClosingSequence = L"]";
			aIsOpening = aView[0] == aOpeningSequence[0];
			aIsClosing = !aIsOpening;
			return true;
		}
		break;
	case L'<':
	case L'>':
		if (IsGroupEnabled(GroupingWordReaderGroup::AngleBracket))
		{
			aGroup = GroupingWordReaderGroup::AngleBracket;
			aOpeningSequence = L"<";
			aClosingSequence = L">";
			aIsOpening = aView[0] == aOpeningSequence[0];
			aIsClosing = !aIsOpening;
			return true;
		}
		break;
	case L'\n':
		if (IsGroupEnabled(GroupingWordReaderGroup::CppSingleLineComment))
		{
			aGroup = GroupingWordReaderGroup::CppSingleLineComment;
			aOpeningSequence = L"//";
			aClosingSequence = L"\n";
			aIsOpening = false;
			aIsClosing = !aIsOpening;
			return true;
		}
		break;
	case L'"':
		if (IsGroupEnabled(GroupingWordReaderGroup::CppLiteralStringWithEscapeCharacters))
		{
			aGroup = GroupingWordReaderGroup::CppLiteralStringWithEscapeCharacters;
			aOpeningSequence = L"\"";
			aClosingSequence = L"\"";
			aIsOpening = true;
			aIsClosing = true;
			return true;
		}
		break;
	default:
		break;
	}

	if (aView.GetLength() >= 2)
	{
		if (aView[0] == L'/')
		{
			if (IsGroupEnabled(GroupingWordReaderGroup::CppSingleLineComment))
			{
				if (aView[1] == L'/')
				{
					aGroup = GroupingWordReaderGroup::CppSingleLineComment;
					aOpeningSequence = L"//";
					aClosingSequence = L"\n";
					aIsOpening = true;
					return true;
				}
			}

			if (IsGroupEnabled(GroupingWordReaderGroup::CppMultiLineComment))
			{
				aGroup = GroupingWordReaderGroup::CppMultiLineComment;
				aOpeningSequence = L"/*";
				aClosingSequence = L"*/";
				aIsOpening = true;
				return true;
			}
		}
		else if (aView[0] == L'*' && aView[1] == L'/')
		{
			if (IsGroupEnabled(GroupingWordReaderGroup::CppMultiLineComment))
			{
				aGroup = GroupingWordReaderGroup::CppMultiLineComment;
				aOpeningSequence = L"/*";
				aClosingSequence = L"*/";
				aIsOpening = false;
				return true;
			}
		}
	}

	return false;
}

bool GroupingWordReader::AnyParentWantsToStopAtSequence(const StringView aSequence, i32& aStopAtDepth, const i32 aCurrentDepth) const
{
	if (!myParentReader)
		return false;

	if (myParentReader->myCurrentClosingSequence == aSequence)
	{
		aStopAtDepth = aCurrentDepth;
		return true;
	}

	return myParentReader->AnyParentWantsToStopAtSequence(aSequence, aStopAtDepth, aCurrentDepth + 1);
}

bool GroupingWordReader::IsGroupExclusive(const GroupingWordReaderGroup aGroup)
{
	switch (aGroup)
	{
	case GroupingWordReaderGroup::CppSingleLineComment:
	case GroupingWordReaderGroup::CppMultiLineComment:
	case GroupingWordReaderGroup::CppLiteralStringWithEscapeCharacters:
		return true;

	default:
		return false;
	}
}
