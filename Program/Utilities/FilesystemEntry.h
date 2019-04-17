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

	std::wstring GetRelativePath(const Directory& aBaseDirectory) const;

protected:
	FileSystemEntry(Directory * aParentDirectory, const StringView & aAbsolutePath);

private:
	String myAbsolutePath;
	Directory * myParentDirectory;
};
