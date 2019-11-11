#include "CorePch.h"
#include "GroupingWordReader.h"

GroupingWordReader::GroupingWordReader(const StringView aContent)
	: Base(aContent)
{
}

bool GroupingWordReader::Next()
{
	const StringView current = NextWord();

	if (!current)
	{
		myIsAtGroup = false;
		return false;
	}

	if (current.Find(myStopAtCharacter) >= 0)
	{
		Rewind(current.GetLength() - 1);
		myIsAtGroup = false;
		return false;
	}

	const Char closingCharacter = GetCorrespondingEndCharacter(current[0]);

	if (closingCharacter == L'\0')
	{
		myCurrentContent = current;
		myIsAtGroup = false;
		return true;
	}
	
	myIsAtGroup = true;
	myCurrentOpeningCharacter = current[0];
	myCurrentClosingCharacter = closingCharacter;
	Rewind(current.GetLength() - 1);
	const i32 previousIndex = GetReadIndex();
	NextWord();

	myCurrentGroupFirstContentLineIndex = CalculateAndGetCurrentLineIndex();

	SetReadIndex(previousIndex);

	const StringView string = GetString();

	i32 index = GetReadIndex();
	const i32 startIndex = index;

	// Include nested groups inside this group, also fixes cases where end character inside of our own group ends the group prematurely
	GroupingWordReader subWordReader(GetString().ChopFromStart(index));
	subWordReader.myStopAtCharacter = closingCharacter;
	while (subWordReader.Next());
	
	index = startIndex + subWordReader.GetReadIndex();
	SetReadIndex(index);

	myCurrentContent = string.Slice(startIndex, index - startIndex - 1);
	myCurrentContent.Trim();
	return true;
}

Char GroupingWordReader::GetCorrespondingEndCharacter(const Char aCharacter)
{
	switch (aCharacter)
	{
	case L'{':
		return L'}';
	case L'(':
		return L')';
	case L'[':
		return L']';
	case L'<':
		return L'>';
	case L'"':
		return L'"';
	default:
		return L'\0';
	}
}
