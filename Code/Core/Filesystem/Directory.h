#pragma once
#include "FilesystemEntry.h"
#include <Containers/Map.h>

class File;

class Directory final : public FileSystemEntry
{
public:
	// TODO: String move semantic helper thing @ep
	Directory(Directory * aParentDirectory, const StringView & aAbsolutePath);
	~Directory();

	FORCEINLINE StringView GetDirectoryName() const { return GetName(); }

	FileSystemEntry* GetEntry(StringView aPath);
	Directory* GetDirectory(StringView aPath);
	File* GetFile(StringView aPath);

	FORCEINLINE bool IsDirectory() const final { return true; }

	template <typename TCallback>
	void IterateFiles(const TCallback& aCallback, const bool aIncludeSubDirectories = false);

	template <typename TCallback>
	void IterateDirectories(const TCallback& aCallback, const bool aIncludeSubDirectories = false);

	template <typename TPredicate>
	File* SelectFirstFile(const TPredicate& aPredicate, const bool aIncludeSubDirectories = false);

	template <typename TPredicate>
	Directory* SelectFirstDirectory(const TPredicate& aPredicate, const bool aIncludeSubDirectories = false);
	
private:
	static File* CastToFile(FileSystemEntry* aPointer);
	FileSystemEntry* GetEntryFromThisDirectory(StringView aEntry);

	Map<String, FileSystemEntry*> myFileSystemEntries;
};

template <typename TCallback>
void Directory::IterateFiles(const TCallback& aCallback, const bool aIncludeSubDirectories)
{
	for (auto && it : myFileSystemEntries)
	{
		if (it.value->IsFile())
		{
			if constexpr (IsSame<decltype(aCallback(*CastToFile(it.value))), bool>)
			{
				if (!aCallback(*CastToFile(it.value)))
					return;
			}
			else
			{
				aCallback(*CastToFile(it.value));
			}
		}
	}

	if (aIncludeSubDirectories == true)
	{
		for (auto && it : myFileSystemEntries)
		{
			if (it.value->IsDirectory())
			{
				static_cast<Directory*>(it.value)->IterateFiles(aCallback, aIncludeSubDirectories);
			}
		}
	}
}

template <typename TCallback>
void Directory::IterateDirectories(const TCallback& aCallback, const bool aIncludeSubDirectories)
{
	for (auto && it : myFileSystemEntries)
	{
		if (it.value->IsDirectory())
		{
			aCallback(*static_cast<Directory*>(it.value));
		}
	}

	if (aIncludeSubDirectories == true)
	{
		for (auto && it : myFileSystemEntries)
		{
			if (it.value->IsDirectory())
			{
				static_cast<Directory*>(it.value)->IterateDirectories(aCallback, aIncludeSubDirectories);
			}
		}
	}
}

template <typename TPredicate>
File* Directory::SelectFirstFile(const TPredicate& aPredicate, const bool aIncludeSubDirectories)
{
	for (auto && it : myFileSystemEntries)
	{
		if (it.value->IsFile())
		{
			if (aPredicate(*CastToFile(it.value)))
			{
				return CastToFile(it.value);
			}
		}
	}

	if (aIncludeSubDirectories == true)
	{
		for (auto && it : myFileSystemEntries)
		{
			if (it.value->IsDirectory())
			{
				return static_cast<Directory*>(it.value)->SelectFirstFile(aPredicate, aIncludeSubDirectories);
			}
		}
	}

	return nullptr;
}

template <typename TPredicate>
Directory* Directory::SelectFirstDirectory(const TPredicate& aPredicate, const bool aIncludeSubDirectories)
{
	for (auto && it : myFileSystemEntries)
	{
		if (it.value->IsDirectory())
		{
			if (aPredicate(*static_cast<Directory*>(it.value)))
			{
				return static_cast<Directory*>(it.value);
			}
		}
	}

	if (aIncludeSubDirectories == true)
	{
		for (auto && it : myFileSystemEntries)
		{
			if (it.value->IsDirectory())
			{
				return static_cast<Directory*>(it.value)->SelectFirstDirectory(aPredicate, aIncludeSubDirectories);
			}
		}
	}

	return nullptr;
}
