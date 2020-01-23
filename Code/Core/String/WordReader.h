#pragma once

/**
Reads one "word" at a time from a source string.
A word is any continuous substring of characters of the same type and may never contain whitespaces with the exception of \n which is treated a standalone word.
The categories are:
	1) Letter, Digits and underscores (e.g. "123_abc" is one word)
	2) Special Character (. , { % " + - etc) (Multiple ones in a row, e.g. "+=" is a single word)
	3) Newline (Consecutive ones are treated as separate words, \r is ignored) (Can be set to ignored)
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
	
	void SetIgnoreNewlines(bool aIgnoreNewline);
	FORCEINLINE bool IsIgnoringNewlines() const { return myIgnoreNewlines; }

private:
	static bool IsWhitespace(Char aCharacter);
	static bool IsLetterDigitOrUnderscore(Char aCharacter);
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

	bool myIgnoreNewlines = true;
};
