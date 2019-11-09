#pragma once

class Directory;

class FileSystemEntry
{
public:
	virtual ~FileSystemEntry();
	virtual bool IsDirectory() const = 0;
	bool IsFile() const;

	FORCEINLINE const String & GetAbsolutePath() const { return myAbsolutePath; }
	Directory * GetParentDirectory() const;
	Directory * GetRootDirectory();

	String GetRelativePath(const Directory& aBaseDirectory) const;

	/* Example outputs: "C:/", "config.json", "My Documents" */
	FORCEINLINE StringView GetName() const { return myName; }

protected:
	FileSystemEntry(Directory * aParentDirectory, const StringView & aAbsolutePath);

private:
	String myAbsolutePath;
	StringView myName;
	Directory * myParentDirectory;
};
