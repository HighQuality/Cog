#include "pch.h"
#include "FileSystemEntry.h"
#include "Directory.h"

	FileSystemEntry::FileSystemEntry(Directory * aParentDirectory, const StringView & aAbsolutePath)
	{
		myParentDirectory = aParentDirectory;
		myAbsolutePath = String(aAbsolutePath);
	}
	
	FileSystemEntry::~FileSystemEntry()
	{
	}
	
	bool FileSystemEntry::IsFile() const
	{
		return !IsDirectory();
	}
	
	Directory * FileSystemEntry::GetParentDirectory() const
	{
		return myParentDirectory;
	}
	
	Directory* FileSystemEntry::GetRootDirectory()
	{
		if (myParentDirectory != nullptr)
		{
			return myParentDirectory->GetRootDirectory();
		}
	
		if (IsDirectory())
		{
			return static_cast<Directory*>(this);
		}
	
		return nullptr;
	}
	
	std::wstring FileSystemEntry::GetRelativePath(const Directory& aBaseDirectory) const
	{
		TODO

		// const StringView baseDirectoryPath = aBaseDirectory.GetAbsolutePath();
		// const StringView ourPath = GetAbsolutePath();
		// if (ourPath.compare(0, baseDirectoryPath.size(), baseDirectoryPath) == 0)
		// {
		// 	return ourPath.substr(baseDirectoryPath.size() + 1);
		// }
		// else
		// {
		// 	abort();
		// }
	}
