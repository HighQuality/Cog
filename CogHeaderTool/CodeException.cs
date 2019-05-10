using System;
using System.Collections.Generic;
using System.Text;

namespace CogHeaderTool
{
    public class CodeException : Exception
    {
        public string SourceFile { get; }
        
        public int SourceFileLine { get; }

        public CodeException(string sourceFile, int sourceFileLine, string message)
            : base(message)
        {
            SourceFile = sourceFile;
            SourceFileLine = sourceFileLine;
        }
    }
}
