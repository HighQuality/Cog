using System;
using System.Collections.Generic;
using System.IO;
using System.Text;

namespace CogHeaderTool
{
    class GeneratedHeader
    {
        public string SourceFile { get; }

        public CogType[] Types { get; }
        
        public string CurrentFileMacroValue { get; }

        public GeneratedHeader(string sourceFile, CogType[] types)
        {
            SourceFile = sourceFile;
            Types = types;

            string fileName = Path.GetFileNameWithoutExtension(SourceFile).ToUpper();
            CurrentFileMacroValue = $"HEADER_{ fileName }";
        }

        public void WriteToDisk()
        {
            StringBuilder fileContents = new StringBuilder();

            WriteCurrentFileMacro(fileContents);

            foreach (var type in Types)
            {
                if (type is CogObjectType objectType)
                {
                    var bodyMacro = new StringBuilder();
                    objectType.GenerateBodyMacro(bodyMacro);
                    
                }
            }
        }

        private void WriteCurrentFileMacro(StringBuilder fileContents)
        {
            fileContents.AppendLine("#undef CURRENT_HEADER_FILE");
            fileContents.AppendLine($"#define CURRENT_HEADER_FILE { CurrentFileMacroValue }");
        }
    }
}
