#pragma once

class CogType
{
public:
	CogType(String aTypeName);
	virtual ~CogType() = default;

	StringView GetTypeName() const { return myTypeName.View(); }

private:
	String myTypeName;
};
