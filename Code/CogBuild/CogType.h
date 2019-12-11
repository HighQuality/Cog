#pragma once

struct DocumentTemplates;

class CogType
{
public:
	CogType(String aTypeName, String aBaseTypeName);
	virtual ~CogType() = default;

	StringView GetTypeName() const { return myTypeName.View(); }
	StringView GetBaseTypeName() const { return myBaseTypeName.View(); }

	bool HasBaseType() const { return myBaseTypeName.GetLength() > 0; }
	
	virtual String GenerateHeaderFileContents(const DocumentTemplates& aTemplates, StringView aGeneratedHeaderIdentifier) const;
	virtual String GenerateSourceFileContents(const DocumentTemplates& aTemplates) const;

private:
	String myTypeName;
	String myBaseTypeName;
};
