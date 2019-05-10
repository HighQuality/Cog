using System;
using System.Collections.Generic;
using System.Text;

namespace CogHeaderTool
{
    static class SpecialCharacters
    {
        public static bool IsSpecial(char character)
        {
            switch (character)
            {
            case '§':
            case '½':
            case '!':
            case '"':
            case '@':
            case '#':
            case '£':
            case '¤':
            case '$':
            case '%':
            case '&':
            case '/':
            case '{':
            case '}':
            case '[':
            case ']':
            case '(':
            case ')':
            case '=':
            case '?':
            case '`':
            case '´':
            case '<':
            case '>':
            case '|':
            case ',':
            case ';':
            case '.':
            case ':':
            case '-':
            case '_':
            case '*':
            case '\'':
            case '^':
            case '¨':
            case '~':
            case '+':
            case '€':
            case '\\':
                return true;

            default:
                return false;
            }
}
    }
}
