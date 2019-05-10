using System;
using System.Collections.Generic;
using System.Text;

namespace CogHeaderTool
{
    public static class Options
    {
        public static string CodeGenOutputDirectory { get; private set; } = "C:/Projects/Cog/temp/GeneratedCode";

        public static bool Parallel { get; private set; } = true;

        public static void Parse(HashSet<string> parameters)
        {
            if (parameters.Remove("-singleThreaded"))
                Parallel = false;
        }
    }
}
