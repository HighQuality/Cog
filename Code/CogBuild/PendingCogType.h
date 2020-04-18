#pragma once

struct PendingCogType
{
	// enum, struct, class etc
	String type;
	// Type inherited from, underlying type for enums?
	String baseType;
	// Type name
	String typeName;
	// Body of type, without curly braces
	String body;
	// Parameter list inside of COGTYPE-macro, without parenthesis
	String parameterList;
};
