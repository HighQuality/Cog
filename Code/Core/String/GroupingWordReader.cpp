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

	for (i32 i = 0; i < current.GetLength(); ++i)
	{
		StringView openingSequence;
		StringView closingSequence;

		bool isOpening;
		bool isClosing;

		if (GetGroupingPair(current.ChopFromStart(i), openingSequence, closingSequence, isOpening, isClosing))
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

	myCurrentGroupFirstContentLineIndex = CalculateAndGetCurrentLineIndex();

	const i32 innerReaderStart = GetReadIndex();
	const StringView innerReaderString = GetString().ChopFromStart(innerReaderStart);

	// Include nested groups inside this group, also fixes cases where end character inside of our own group ends the group prematurely
	GroupingWordReader innerReader(innerReaderString);
	innerReader.CopySettingsFrom(*this);
	innerReader.SetIgnoreNewlines(false);
	innerReader.myParentReader = this;

	while (innerReader.Next())
	{ }

	const i32 innerReaderStop = innerReaderStart + innerReader.GetReadIndex();

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

bool GroupingWordReader::GetGroupingPair(StringView aView, StringView& aOpeningSequence, StringView& aClosingSequence, bool &aIsOpening, bool& aIsClosing) const
{
	if (aView.GetLength() == 0)
		return false;

	switch (aView[0])
	{
	case L'{':
	case L'}':
		if (IsGroupEnabled(GroupingWordReaderGroup::CurlyBraces))
		{
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
					aOpeningSequence = L"//";
					aClosingSequence = L"\n";
					aIsOpening = true;
					return true;
				}
			}

			if (IsGroupEnabled(GroupingWordReaderGroup::CppMultiLineComment))
			{
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
