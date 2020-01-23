#include "CorePch.h"
#include "WordReader.h"

WordReader::WordReader(const StringView aContent)
{
	myString = aContent;
}

StringView WordReader::NextWord()
{
	const i32 contentLength = myString.GetLength();

	if (myCurrentIndex >= contentLength)
	{
		myPreviouslyReturnedWordIndex = myCurrentIndex;
		return StringView();
	}

	while (IsWhitespace(myString[myCurrentIndex]))
	{
		++myCurrentIndex;

		if (myCurrentIndex >= contentLength)
		{
			myPreviouslyReturnedWordIndex = myCurrentIndex;
			return StringView();
		}
	}

	const auto continueWhile = [this, contentLength](bool(*const aPredicate)(Char)) -> StringView
	{
		const i32 startIndex = myCurrentIndex;

		while (myCurrentIndex < contentLength)
		{
			if (!aPredicate(myString[myCurrentIndex]))
				break;

			++myCurrentIndex;
		}

		return myString.Slice(startIndex, myCurrentIndex - startIndex);
	};

	myPreviouslyReturnedWordIndex = myCurrentIndex;

	const StringView alphanumericWord = continueWhile([](const Char aCharacter) { return IsLetterDigitOrUnderscore(aCharacter); });
	
	if (alphanumericWord.GetLength() > 0)
		return alphanumericWord;

	const StringView specialCharacterWord = continueWhile([](const Char aCharacter) { return IsSpecialCharacter(aCharacter); });

	if (specialCharacterWord.GetLength() > 0)
		return specialCharacterWord;

	if (myString[myCurrentIndex] == L'\n')
	{
		++myCurrentIndex;

		if (myIgnoreNewlines)
			return NextWord();

		return L"\n";
	}

	FATAL(L"Unexpected character '%' (%)", static_cast<i32>(myString[myCurrentIndex]), myString[myCurrentIndex]);
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

void WordReader::SetIgnoreNewlines(const bool aIgnoreNewline)
{
	myIgnoreNewlines = aIgnoreNewline;
}

void WordReader::RefreshLineAndColumnIndex()
{
	const i32 targetIndex = myPreviouslyReturnedWordIndex;
	bring_into_scope(cachedIndex, myCachedLineAndColumnIndex);
	bring_into_scope(columnIndex, myCurrentColumnIndex);
	bring_into_scope(lineIndex, myCurrentLineIndex);

	if (cachedIndex < targetIndex)
	{
		while (cachedIndex < targetIndex)
		{
			const Char previousCharacter = myString.TryGet(cachedIndex, L'\0');
			++cachedIndex;
			++columnIndex;

			if (previousCharacter == L'\n')
			{
				columnIndex = 0;
				++lineIndex;
			}
		}
	}
	else if (cachedIndex > targetIndex)
	{
		while (cachedIndex > targetIndex)
		{
			--cachedIndex;
			--columnIndex;

			if (myString[cachedIndex] == L'\n')
			{
				columnIndex = CalculateColumnIndexAt(cachedIndex);
				--lineIndex;
			}
		}
	}
}

i32 WordReader::CalculateColumnIndexAt(const i32 aIndex) const
{
	i32 column = 0;
	i32 current = aIndex - 1;

	while (current >= 0)
	{
		if (myString[current] == L'\n')
			break;

		--current;
		++column;
	}

	return column;
}

bool WordReader::IsWhitespace(const Char aCharacter)
{
	switch (aCharacter)
	{
	case L'\t':
	case L' ':
	case L'\r':
		return true;

	default:
		return false;
	}
}

bool WordReader::IsLetterDigitOrUnderscore(const Char aCharacter)
{
	return iswalnum(aCharacter) || aCharacter == L'_';
}

bool WordReader::IsControlCharacter(const Char aCharacter)
{
	return iswcntrl(aCharacter);
}

bool WordReader::IsSpecialCharacter(const Char aCharacter)
{
	if (IsLetterDigitOrUnderscore(aCharacter))
		return false;

	if (IsWhitespace(aCharacter))
		return false;

	if (IsControlCharacter(aCharacter))
		return false;

	return true;
}
