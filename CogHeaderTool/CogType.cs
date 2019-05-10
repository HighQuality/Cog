using System;
using System.Collections.Generic;
using System.Text;

namespace CogHeaderTool
{
    public class CogType
    {
        public string Name { get; }
        
        public string SourceFile { get; }
        
        public int SourceFileLine { get; }

        public CogType(string name, string sourceFile, int sourceFileLine)
        {
            Name = name;

            SourceFile = sourceFile;
            SourceFileLine = sourceFileLine;
        }

        public virtual void ResolveDependencies(Dictionary<string, CogType> typeMap)
        {
        }
        
        protected void ThrowErrorAtDeclaration(string message)
        {
            throw new CodeException(SourceFile, SourceFileLine, $"{Name}: {message}");
        }
    }
}
