#pragma once

/**
Reads one "word" at a time from a source string.
A word is any continuous substring of characters of the same type and may never contain whitespaces.
The categories are:
	1) Letter and Digits (e.g. "123abc" is one word)
	2) Special Character (. , { % " + - etc) (Multiple ones in a row, e.g. "+=" is a single word)
*/
class WordReader
{
public:
	explicit WordReader(StringView aContent);

	StringView NextWord();
	const StringView& GetString() const { return myString; }

	void SetReadIndex(i32 aNewIndex);
	i32 GetReadIndex() const { return myCurrentIndex; }

	void Rewind(i32 aOffset);
	void Forward(i32 aOffset);

private:
	static bool IsWhitespace(Char aCharacter);
	static bool IsLetterOrDigit(Char aCharacter);
	static bool IsNewline(Char aCharacter);
	static bool IsControlCharacter(Char aCharacter);
	static bool IsSpecialCharacter(Char aCharacter);

	StringView myString;
	i32 myCurrentIndex = 0;
};
