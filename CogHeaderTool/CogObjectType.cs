using System;
using System.Collections.Generic;
using System.Text;

namespace CogHeaderTool
{
    public class CogObjectType : CogType
    {
        public string BaseClassName { get; }

        public CogObjectType BaseClass { get; private set; }

        private List<GeneratedBodyCode> generatedBodyCode = null;

        public CogObjectType(string name, string baseClassName, string sourceFile, int sourceFileLine)
            : base(name, sourceFile, sourceFileLine)
        {
            BaseClassName = baseClassName;
        }

        public override void ResolveDependencies(Dictionary<string, CogType> typeMap)
        {
            base.ResolveDependencies(typeMap);

            if (!string.IsNullOrEmpty(BaseClassName))
                ResolveBaseClass(typeMap);
        }

        private void ResolveBaseClass(Dictionary<string, CogType> typeMap)
        {
            if (typeMap.TryGetValue(BaseClassName, out var baseClass))
            {
                if (baseClass != null)
                {
                    if (!(baseClass is CogObjectType))
                        ThrowErrorAtDeclaration($"Non-object base type {BaseClassName} ({BaseClassName} is a {baseClass.GetType().Name}");

                    BaseClass = (CogObjectType)baseClass;
                }
            }

            if (BaseClass == null)
                ThrowErrorAtDeclaration($"Class {Name} derives");
        }

        public void AddGeneratedBodyCode(string code)
        {
            if (generatedBodyCode == null)
                generatedBodyCode = new List<GeneratedBodyCode>();

            generatedBodyCode.Add(new GeneratedBodyCode(code));
        }

        public void GenerateBodyMacro(StringBuilder result)
        {
            if (generatedBodyCode != null)
            {
                foreach (var bodyCode in generatedBodyCode)
                    bodyCode.GenerateBodyMacro(result);
            }
        }
    }
}
