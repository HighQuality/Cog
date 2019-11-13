#pragma once

class CogType
{
public:
	CogType(String aTypeName);
	virtual ~CogType() = default;

	StringView GetTypeName() const { return myTypeName.View(); }

	virtual String GenerateHeaderFileContents() const;
	virtual String GenerateSourceFileContents() const;

private:
	String myTypeName;
};
