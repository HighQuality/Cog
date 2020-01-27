#pragma once

struct DocumentTemplates;

class CogType
{
public:
	CogType(String aAbsoluteFilePath, i32 aDeclarationLine, String aTypeName, String aBaseTypeName);
	virtual ~CogType() = default;

	StringView GetTypeName() const { return myTypeName.View(); }
	StringView GetBaseTypeName() const { return myBaseTypeName.View(); }

	bool HasBaseType() const { return myBaseTypeName.GetLength() > 0; }
	virtual bool ResolveDependencies(const Map<String, CogType*>& aCogTypes);

	virtual String GenerateHeaderFileContents(const DocumentTemplates& aTemplates, StringView aGeneratedHeaderIdentifier) const;
	virtual String GenerateSourceFileContents(const DocumentTemplates& aTemplates) const;

	String GetDeclarationLocation() const;

protected:
	FORCEINLINE const CogType* GetBaseType() const { return myBaseType; }

private:
	String myAbsoluteFilePath;
	String myTypeName;
	String myBaseTypeName;
	CogType* myBaseType = nullptr;
	i32 myDeclarationLine;
};
