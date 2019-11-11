#pragma once

/**
Reads one "word" at a time from a source string.
A word is any continuous substring of characters of the same type and may never contain whitespaces.
The categories are:
	1) Letter, Digits and underscores (e.g. "123_abc" is one word)
	2) Special Character (. , { % " + - etc) (Multiple ones in a row, e.g. "+=" is a single word)
*/
class WordReader
{
public:
	WordReader() = default;
	explicit WordReader(StringView aContent);

	StringView NextWord();
	StringView GetString() const { return myString; }

	void SetReadIndex(i32 aNewIndex);
	i32 GetReadIndex() const { return myCurrentIndex; }

	void Rewind(i32 aOffset);
	void Forward(i32 aOffset);

	/** Current refers to the word that was previously returned by NextWord */
	i32 CalculateAndGetCurrentLineIndex() { RefreshLineAndColumnIndex(); return myCurrentLineIndex; }
	/** Current refers to the word that was previously returned by NextWord */
	i32 CalculateAndGetCurrentColumnIndex() { RefreshLineAndColumnIndex(); return myCurrentColumnIndex; }

private:
	static bool IsWhitespace(Char aCharacter);
	static bool IsLetterDigitOrUnderscore(Char aCharacter);
	static bool IsNewline(Char aCharacter);
	static bool IsControlCharacter(Char aCharacter);
	static bool IsSpecialCharacter(Char aCharacter);

	void RefreshLineAndColumnIndex();
	i32 CalculateColumnIndexAt(i32 aIndex) const;

	StringView myString;
	i32 myCurrentIndex = 0;

	i32 myPreviouslyReturnedWordIndex = -1;
	i32 myCachedLineAndColumnIndex = 0;
	i32 myCurrentLineIndex = 0;
	i32 myCurrentColumnIndex = 0;
};
