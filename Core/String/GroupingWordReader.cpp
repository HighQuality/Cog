#include "pch.h"
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

	const StringView string = GetString();

	i32 index = GetReadIndex();
	const i32 startIndex = index;

	while (index < string.GetLength() && string[index] != closingCharacter)
		++index;

	SetReadIndex(index + 1);

	myCurrentContent = string.Slice(startIndex, index - startIndex);
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