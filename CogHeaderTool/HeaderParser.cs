using System;
using System.Collections.Generic;
using System.IO;
using System.Text;
using System.Threading.Tasks;

namespace CogHeaderTool
{
    class HeaderParser
    {
        public string FilePath { get; }

        public string FileContents { get; }

        private char CurrentCharacter { get { return FileContents[parseIndex]; } }

        public int CurrentLine { get; private set; }

        private int parseIndex = 0;

        public static async Task<CogType[]> ParseFile(string headerFile)
        {
            string fileContents = await File.ReadAllTextAsync(headerFile);

            var headerParser = new HeaderParser(headerFile, fileContents);

            return headerParser.Parse();
        }

        private HeaderParser(string filePath, string fileContents)
        {
            FilePath = filePath;
            FileContents = fileContents;
        }

        private CogType[] Parse()
        {
            var results = new List<CogType>();

            string current = null;

            do
            {
                try
                {
                    current = MoveForward();

                    if (current == "COG_IGNORE_HEADER")
                    {
                        break;
                    }
                    else if (current == "GENERATED_BODY")
                    {
                        Console.WriteLine($"GENERATED_BODY on {FilePath}:{CurrentLine}");

                        Expect("(");
                        Expect(")");
                    }
                }
                catch (CodeException ex)
                {
                    Console.WriteLine($"{ex.SourceFile}:{ex.SourceFileLine} {ex.Message}");
                }
            }
            while (!string.IsNullOrEmpty(current));

            return results.ToArray();
        }

        // TODO: Make return spans into the string instead of new copies of substrings
        private string MoveForward()
        {
            for (; ; )
            {
                SkipWhiteSpaces();

                if (parseIndex >= FileContents.Length)
                    return "";

                if (SpecialCharacters.IsSpecial(CurrentCharacter))
                {
                    if (CurrentCharacter == '"')
                    {
                        // TODO: String Parsing
                        ++parseIndex;
                        return FileContents.Substring(parseIndex - 1, 1);
                    }
                    else
                    {
                        ++parseIndex;
                        return FileContents.Substring(parseIndex - 1, 1);
                    }
                }
                else if (char.IsLetterOrDigit(CurrentCharacter) || CurrentCharacter == '_')
                {
                    int start = parseIndex;

                    if (char.IsDigit(CurrentCharacter))
                    {
                        while (parseIndex < FileContents.Length)
                        {
                            if (CurrentCharacter != '.' && !char.IsDigit(CurrentCharacter))
                                break;
                            ++parseIndex;
                        }

                        return FileContents.Substring(start, parseIndex - start);
                    }
                    else
                    {
                        while (parseIndex < FileContents.Length)
                        {
                            if (!char.IsLetterOrDigit(CurrentCharacter) && CurrentCharacter != '_')
                                break;
                            ++parseIndex;
                        }

                        return FileContents.Substring(start, parseIndex - start);
                    }
                }
                else
                {
                    if (CurrentCharacter == '\n')
                    {
                        ++CurrentLine;
                        ++parseIndex;
                    }
                    else
                    {
                        switch (CurrentCharacter)
                        {
                            // Ignored characters
                            case '\r':
                                break;

                            default:
                                throw new CodeException(FilePath, CurrentLine, $"Unknown character '{CurrentCharacter}' ({(int)CurrentCharacter})");
                        }
                    }
                }
            }
        }

        private string PeekForward()
        {
            int previousIndex = parseIndex;
            var token = MoveForward();
            parseIndex = previousIndex;
            return token;
        }

        private void SkipWhiteSpaces()
        {
            while (parseIndex < FileContents.Length && char.IsWhiteSpace(CurrentCharacter) && CurrentCharacter != '\n')
                ++parseIndex;
        }

        private bool IsFollowedBy(string token)
        {
            if (string.IsNullOrEmpty(token))
                return true;

            SkipWhiteSpaces();

            if (parseIndex + token.Length > FileContents.Length)
                return false;

            return string.Compare(FileContents, parseIndex, token, 0, token.Length) == 0;
        }

        private bool Eat(string token)
        {
            if (!IsFollowedBy(token))
                return false;

            if (MoveForward() != token)
                throw new CodeException(FilePath, CurrentLine, $"Invalid token to eat {token}");

            return true;
        }

        private void Expect(string token)
        {
            if (!Eat(token))
                throw new CodeException(FilePath, CurrentLine, $"Expected token '{token}'");
        }
    }
}
