using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace CogHeaderTool
{
    class GeneratedBodyCode
    {
        public string Declaration { get; private set; }

        public GeneratedBodyCode(string declaration)
        {
            Declaration = declaration.Replace("\n", "\\\n");
        }
        
        public void GenerateBodyMacro(StringBuilder bodyMacro)
        {
            bodyMacro.Append(Declaration);
            bodyMacro.Append('\n');
        }
    }
}
