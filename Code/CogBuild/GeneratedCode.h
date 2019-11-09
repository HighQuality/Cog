#pragma once

struct GeneratedFunctionParameter
{
	String Type;
	String Name;
};

struct GeneratedFunction
{
	String functionName;
	Array<GeneratedFunctionParameter> parameterDeclarations;
	bool isInlined = false;
};

class GeneratedCode
{
public:
	GeneratedCode(StringView aMainFileName);

	void WriteFiles(const StringView aOutputDirectory);

private:
	Map<String, u8> mySourceFileIncludes;
	Array<GeneratedFunction> myGlobalFunctions;
	String myFileName;
};
