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

	if (myParentReader || myStopAtSequence)
	{
		i32 stopIndex = MaxOf<i32>;

		// In cases such as "if (a < b)" we need to respect our parents' stop characters
		if (myParentReader)
		{
			for (i32 i = 0; i < current.GetLength(); ++i)
			{
				StringView closingSequence;

				if (GetCorrespondingOpeningSequence(current.ChopFromStart(i), closingSequence) && AnyParentWantsToStopAtSequence(closingSequence))
				{
					stopIndex = i;
					break;
				}
			}
		}

		if (myStopAtSequence)
		{
			i32 j = Min(stopIndex, current.GetLength());

			for (i32 i = 0; i < j; ++i)
			{
				if (current.ChopFromStart(i).StartsWith(myStopAtSequence))
				{
					stopIndex = i;
					break;
				}
			}
		}

		if (stopIndex != MaxOf<i32>)
		{
			if (stopIndex > 0)
			{
				myCurrentContent = current.SliceFromStart(stopIndex);

				const i32 rewindAmount = current.GetLength() - stopIndex - 1;
				Rewind(rewindAmount);
				return true;
			}

			// Rewind ourself so our user can continue reading at the appropriate location
			const i32 rewindAmount = current.GetLength() - stopIndex;
			Rewind(rewindAmount);
			return false;
		}
	}

	for (i32 i = 0; i < current.GetLength(); ++i)
	{
		StringView openingSequence;

		if (const StringView closingSequence = GetCorrespondingClosingSequence(current.ChopFromStart(i), openingSequence))
		{
			if (i > 0)
			{
				myCurrentContent = current.SliceFromStart(i);

				const i32 rewindAmount = current.GetLength() - i - 1;
				Rewind(rewindAmount);
				return true;
			}

			myIsAtGroup = true;
			myCurrentOpeningSequence = openingSequence;
			myCurrentClosingSequence = closingSequence;

			const i32 rewindAmount = current.GetLength() - openingSequence.GetLength();
			Rewind(rewindAmount);
			break;
		}
	}

	if (!myIsAtGroup)
	{
		myCurrentContent = current;
		return true;
	}

	// Calculate content line index, PERF: disable
	{
		const i32 previousIndex = GetReadIndex();
		NextWord();

		myCurrentGroupFirstContentLineIndex = CalculateAndGetCurrentLineIndex();

		SetReadIndex(previousIndex);
	}

	const i32 innerReaderStart = GetReadIndex();
	const StringView stringFromGroupContentStart = GetString().ChopFromStart(innerReaderStart);

	// Include nested groups inside this group, also fixes cases where end character inside of our own group ends the group prematurely
	GroupingWordReader innerReader(stringFromGroupContentStart);
	innerReader.CopySettingsFrom(*this);
	innerReader.myParentReader = this;
	innerReader.myStopAtSequence = myCurrentClosingSequence;

	bool hadAnyContent = false;

	while (innerReader.Next())
	{
		hadAnyContent = true;
	}


	const i32 innerReaderStop = GetReadIndex() + innerReader.GetReadIndex();
	SetReadIndex(innerReaderStop + 1);

	if (hadAnyContent)
	{
		myCurrentContent = GetString().Slice(innerReaderStart, innerReaderStop - innerReaderStart);
	}
	else
	{
		myCurrentContent = GetString().Slice(innerReaderStart - myCurrentOpeningSequence.GetLength(), innerReaderStop - innerReaderStart + myCurrentOpeningSequence.GetLength());

		myIsAtGroup = false;
	}

	myCurrentContent.Trim();

	return true;
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
}

StringView GroupingWordReader::GetCorrespondingClosingSequence(const StringView aView, StringView& aOpeningSequence) const
{
	if (aView.GetLength() == 0)
		return StringView();

	switch (aView[0])
	{
	case L'{':
		if (IsGroupEnabled(GroupingWordReaderGroup::CurlyBraces))
		{
			aOpeningSequence = L"{";
			return L"}";
		}
		break;
	case L'(':
		if (IsGroupEnabled(GroupingWordReaderGroup::Parenthesis))
		{
			aOpeningSequence = L"(";
			return L")";
		}
		break;
	case L'[':
		if (IsGroupEnabled(GroupingWordReaderGroup::SquareBracket))
		{
			aOpeningSequence = L"[";
			return L"]";
		}
		break;
	case L'<':
		if (IsGroupEnabled(GroupingWordReaderGroup::AngleBracket))
		{
			aOpeningSequence = L"<";
			return L">";
		}
		break;
	case L'"':
		if (IsGroupEnabled(GroupingWordReaderGroup::CppLiteralStringWithEscapeCharacters))
		{
			aOpeningSequence = L"\"";
			return L"\"";
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
					return L"\n";
				}
			}

			if (IsGroupEnabled(GroupingWordReaderGroup::CppMultiLineComment))
			{
				if (aView[1] == L'*')
				{
					aOpeningSequence = L"/*";
					return L"*/";
				}
			}
		}
	}

	return StringView();
}

StringView GroupingWordReader::GetCorrespondingOpeningSequence(const StringView aView, StringView& aClosingSequence) const
{
	if (aView.GetLength() == 0)
		return StringView();

	switch (aView[0])
	{
	case L'}':
		if (IsGroupEnabled(GroupingWordReaderGroup::CurlyBraces))
		{
			aClosingSequence = L"}";
			return L"{";
		}
		break;
	case L')':
		if (IsGroupEnabled(GroupingWordReaderGroup::Parenthesis))
		{
			aClosingSequence = L")";
			return L"(";
		}
		break;
	case L']':
		if (IsGroupEnabled(GroupingWordReaderGroup::SquareBracket))
		{
			aClosingSequence = L"]";
			return L"[";
		}
		break;
	case L'>':
		if (IsGroupEnabled(GroupingWordReaderGroup::AngleBracket))
		{
			aClosingSequence = L">";
			return L"<";
		}
		break;
	case L'"':
		if (IsGroupEnabled(GroupingWordReaderGroup::CppLiteralStringWithEscapeCharacters))
		{
			aClosingSequence = L"\"";
			return L"\"";
		}
		break;
	case L'\n':
		if (IsGroupEnabled(GroupingWordReaderGroup::CppSingleLineComment))
		{
			aClosingSequence = L"\n";
			return L"//";
		}
		break;
	default:
		break;
	}

	if (IsGroupEnabled(GroupingWordReaderGroup::CppMultiLineComment))
	{
		if (aView.GetLength() >= 2)
		{
			if (aView[0] == L'*' && aView[1] == L'/')
			{
				aClosingSequence = L"*/";
				return L"/*";
			}
		}
	}

	return StringView();
}

bool GroupingWordReader::AnyParentWantsToStopAtSequence(const StringView aSequence) const
{
	if (!myParentReader)
		return false;

	if (myParentReader->myStopAtSequence == aSequence)
		return true;

	return myParentReader->AnyParentWantsToStopAtSequence(aSequence);
}
