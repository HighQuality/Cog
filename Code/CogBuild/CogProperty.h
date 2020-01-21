#pragma once

struct CogProperty
{
	String propertyName;
	String propertyType;

	bool directAccess = false;
	bool publicRead = false;
};
