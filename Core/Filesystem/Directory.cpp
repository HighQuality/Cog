#include "pch.h"
#include "Directory.h"
#include "File.h"
#include <Shlwapi.h>
#include <memory>
#include <string>
#pragma comment ( lib, "Shlwapi.lib" )

	Directory::Directory(Directory * aParentDirectory, const StringView & aAbsolutePath)
		: FileSystemEntry(aParentDirectory, aAbsolutePath)
	{
		StringView path = GetAbsolutePath();
		const Char lastCharacter = path.Last();
		if (lastCharacter == L'\\' || lastCharacter == '/')
			path = aAbsolutePath.ChopFromEnd(1);
	
		String searchPatern(path);
		searchPatern.Add(L'\\');
		searchPatern.Add(L'*');
		searchPatern.CheckEndsWithZero();
	
		WIN32_FIND_DATAW data;
		const HANDLE findHandle = FindFirstFileW(searchPatern.GetData(), &data);
		if (findHandle != INVALID_HANDLE_VALUE)
		{
			do
			{
				if (wcscmp(data.cFileName, L".") == 0 || wcscmp(data.cFileName, L"..") == 0)
				{
					continue;
				}
	
				std::unique_ptr<FileSystemEntry> entry;

				const StringView fileName = data.cFileName;
				String newPath(path);
				newPath.Add(L'\\');
				newPath.Append(fileName);
	
				if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					entry = std::make_unique<Directory>(this, newPath);
				}
				else
				{
					u64 fileSize = static_cast<u64>(data.nFileSizeHigh) * (static_cast<size_t>(MAXDWORD) + 1LL);
					fileSize += data.nFileSizeLow;
					u64 lastWriteTime = static_cast<u64>(data.ftLastWriteTime.dwHighDateTime) << 32 | data.ftLastWriteTime.dwLowDateTime;
					entry = std::make_unique<File>(*this, newPath, fileSize, lastWriteTime);
				}
				
				myFileSystemEntries.Add(fileName.ToLower(), entry.release());
	
			} while (FindNextFileW(findHandle, &data));
			FindClose(findHandle);
		}
	}
	
	Directory::~Directory()
	{
	}
	
	FileSystemEntry* Directory::GetEntry(StringView aPath)
	{
		if (aPath.GetLength() > 0 && (aPath[0] == '\\' || aPath[0] == L'/'))
			aPath = aPath.ChopFromStart(1);
	
		for (i32 i = 0; i < aPath.GetLength(); ++i)
		{
			if (aPath[i] == L'\\' || aPath[i] == L'/')
			{
				FileSystemEntry * ourEntry = GetEntryFromThisDirectory(aPath.ChopFromStart(i));
	
				if (ourEntry != nullptr)
				{
					return static_cast<Directory*>(ourEntry)->GetEntry(aPath.ChopFromStart(i + 1));
				}
	
				return nullptr;
			}
	
			// Handle drive paths seperately
			if (aPath[i] == L':')
			{
				return GetEntryDrivePath(aPath);
			}
		}
	
		return GetEntryFromThisDirectory(aPath);
	}
	
	Directory* Directory::GetDirectory(StringView aPath)
	{
		FileSystemEntry * entry = GetEntry(aPath);
	
		if (entry != nullptr && entry->IsDirectory())
		{
			return static_cast<Directory*>(entry);
		}
	
		return nullptr;
	}
	
	File* Directory::GetFile(StringView aPath)
	{
		FileSystemEntry * entry = GetEntry(aPath);
	
		if (entry != nullptr && entry->IsFile())
		{
			return static_cast<File*>(entry);
		}
	
		return nullptr;
	}
	
	bool Directory::IsDirectory() const
	{
		return true;
	}
	
	File* Directory::CastToFile(FileSystemEntry* aPointer)
	{
		return static_cast<File*>(aPointer);
	}
	
	FileSystemEntry * Directory::GetEntryFromThisDirectory(StringView aEntry)
	{
		if (aEntry == L"." || aEntry == L"")
			return this;
		if (aEntry == L"..")
			return GetParentDirectory();
	
		FileSystemEntry ** ptr = myFileSystemEntries.Find(aEntry.ToLower());
		if (ptr)
			return *ptr;
		return nullptr;
	}
	
	FileSystemEntry* Directory::GetEntryDrivePath(StringView aDrivePath)
	{
		abort();
		// Directory * rootDirectory = GetRootDirectory();
		// 
		// if (rootDirectory == this || rootDirectory == nullptr)
		// {
		// 	std::wstring s = aDrivePath;
		// 	for (size_t i = 0; i < aDrivePath.size(); i++)
		// 	{
		// 		s[i] = towlower(s[i] == L'/' ? L'\\' : s[i]);
		// 	}
		// 	std::unique_ptr<wchar_t[]> buffer(new wchar_t[MAX_PATH]);
		// 	BOOL success = PathCanonicalizeW(buffer.get(), s.c_str());
		// 	if (success == false)
		// 	{
		// 		// Error("PathCanonicalizeW failed");
		// 		abort();
		// 	}
		// 	std::wstring str = buffer.get();
		// 	std::wstring ourPath = GetAbsolutePath();
		// 	for (size_t i = 0; i < ourPath.size(); ++i)
		// 	{
		// 		ourPath[i] = towlower(ourPath[i]);
		// 	}
		// 
		// 	if (str.compare(0, ourPath.length(), ourPath) == 0)
		// 	{
		// 		return GetEntry(str.substr(ourPath.length()));
		// 	}
		// 
		// 	return nullptr;
		// }
		// 
		// return rootDirectory->GetEntryDrivePath(aDrivePath);
	}
