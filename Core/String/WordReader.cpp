#include "pch.h"
#include "WordReader.h"

WordReader::WordReader(const StringView aContent)
{
	myString = aContent;
}

StringView WordReader::NextWord()
{
	bring_into_scope(index, myCurrentIndex);
	const i32 contentLength = myString.GetLength();

	if (index >= contentLength)
		return StringView();

	while (IsWhitespace(myString[index]))
	{
		++index;

		if (index >= contentLength)
			return StringView();
	}

	const auto continueWhile = [this, &_index = index, contentLength](bool(*const aPredicate)(Char)) -> StringView
	{
		bring_into_scope(index, _index);
		
		const i32 startIndex = index;

		while (index < contentLength)
		{
			if (!aPredicate(myString[index]))
				break;

			++index;
		}

		return myString.Slice(startIndex, index - startIndex);
	};

	const StringView alphanumericWord = continueWhile([](const Char aCharacter) { return IsLetterOrDigit(aCharacter); });
	
	if (alphanumericWord.GetLength() > 0)
		return alphanumericWord;

	const StringView specialCharacterWord = continueWhile([](const Char aCharacter) { return IsSpecialCharacter(aCharacter); });

	if (specialCharacterWord.GetLength() > 0)
		return specialCharacterWord;

	FATAL(L"Unexpected end of content");
}

void WordReader::SetReadIndex(i32 aNewIndex)
{
	myCurrentIndex = Clamp(aNewIndex, 0, myString.GetLength());
}

void WordReader::Rewind(const i32 aOffset)
{
	if (aOffset == 0)
		return;

	if (aOffset < 0)
	{
		Forward(-aOffset);
		return;
	}

	myCurrentIndex -= aOffset;

	if (myCurrentIndex < 0)
		myCurrentIndex = 0;
}

void WordReader::Forward(const i32 aOffset)
{
	if (aOffset == 0)
		return;

	if (aOffset < 0)
	{
		Rewind(-aOffset);
		return;
	}

	myCurrentIndex += aOffset;

	if (myCurrentIndex > myString.GetLength())
		myCurrentIndex = myString.GetLength();
}

bool WordReader::IsWhitespace(const Char aCharacter)
{
	return iswspace(aCharacter);
}

bool WordReader::IsLetterOrDigit(const Char aCharacter)
{
	return iswalnum(aCharacter);
}

bool WordReader::IsNewline(const Char aCharacter)
{
	switch (aCharacter)
	{
	case L'\r':
	case L'\n':
		return true;

	default:
		return false;
	}
}

bool WordReader::IsControlCharacter(const Char aCharacter)
{
	return iswcntrl(aCharacter);
}

bool WordReader::IsSpecialCharacter(const Char aCharacter)
{
	if (IsLetterOrDigit(aCharacter))
		return false;

	if (IsWhitespace(aCharacter))
		return false;

	if (IsControlCharacter(aCharacter))
		return false;

	return true;
}
