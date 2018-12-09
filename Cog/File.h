#pragma once
#include "FilesystemEntry.h"

class Directory;

class File : public FileSystemEntry
{
public:
	File(Directory & aParentDirectory, const StringView & aAbsolutePath, const size_t aFileSize, const uint64_t aLastWriteTime);
	File(const File&) = delete;
	File & operator=(const File&) = delete;
	~File();

	bool IsDirectory() const override final;

	u64 GetFileSize() const;
	u64 GetLastWriteTime() const;

	// Includes extension but not path (eg "test.dds")
	const StringView & GetFilename() const { return myFilename; }
	// Does not include path (eg "test")
	const StringView & GetFilenameWithoutExtension() const { return myFilenameWithoutExtension; }
	// Includes dot (eg ".dds")
	const StringView & GetExtension() const { return myExtension; }

	Array<u8> ReadFile() const;
	String ReadString() const;

private:
	StringView myFilename;
	StringView myFilenameWithoutExtension;
	StringView myExtension;
	uint64_t myLastWriteTime;
	size_t myFileSize;
};
