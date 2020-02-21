#pragma once

struct CogProperty
{
	String propertyName;
	String propertyType;

	String constructionArguments;
	String defaultAssignment;

	bool directAccess = false;
	bool publicRead = false;
	bool zeroMemory = true;
};
