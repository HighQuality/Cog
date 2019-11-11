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
