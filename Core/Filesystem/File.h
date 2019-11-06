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

	FORCEINLINE bool IsDirectory() const final { return false; }

	u64 GetFileSize() const;
	u64 GetLastWriteTime() const;

	// Includes extension but not path (eg "test.dds")
	FORCEINLINE const StringView & GetFilename() const { return GetName(); }
	// Does not include path (eg "test")
	FORCEINLINE const StringView & GetFilenameWithoutExtension() const { return myFilenameWithoutExtension; }
	// Includes dot (eg ".dds")
	FORCEINLINE const StringView & GetExtension() const { return myExtension; }

	Array<u8> ReadFile() const;
	String ReadString() const;

private:
	StringView myFilenameWithoutExtension;
	StringView myExtension;
	uint64_t myLastWriteTime;
	size_t myFileSize;
};
