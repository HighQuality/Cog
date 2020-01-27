#pragma once

struct CogProperty
{
	String propertyName;
	String propertyType;

	String defaultValue;

	bool directAccess = false;
	bool publicRead = false;
	bool zeroMemory = true;
};
