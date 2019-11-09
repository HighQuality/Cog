#include "CorePch.h"
#include "FileSystemEntry.h"
#include "Directory.h"
#include <Shlwapi.h>

FileSystemEntry::FileSystemEntry(Directory * aParentDirectory, const StringView & aAbsolutePath)
{
	myParentDirectory = aParentDirectory;
	myAbsolutePath = String(aAbsolutePath);

	if (myAbsolutePath.ClampedSliceFromEnd(1) == L"\\" || myAbsolutePath.ClampedSliceFromEnd(1) == L"/")
		myAbsolutePath.Pop();

	for (i32 i = myAbsolutePath.GetLength() - 1; i >= 0; --i)
	{
		if (myAbsolutePath[i] == L'\\' || myAbsolutePath[i] == L'/')
		{
			myName = myAbsolutePath.ChopFromStart(i + 1);
			break;
		}
	}

	if (myName.GetLength() == 0)
		myName = myAbsolutePath;
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

String FileSystemEntry::EvaluatePath(const String aPath)
{
	std::unique_ptr<wchar_t[]> buffer(new wchar_t[MAX_PATH]);

	BOOL success = PathCanonicalizeW(buffer.get(), aPath.GetData());
	if (!success)
		FATAL("PathCanonicalizeW failed");

	return String(buffer.get());
}
	
String FileSystemEntry::GetRelativePath(const Directory& aBaseDirectory) const
{
	const StringView baseDirectoryPath = aBaseDirectory.GetAbsolutePath();
	const StringView ourPath = GetAbsolutePath();
	if (ourPath.ClampedSliceFromStart(baseDirectoryPath.GetLength()) == baseDirectoryPath)
	{
		return String(ourPath.ChopFromStart(baseDirectoryPath.GetLength() + 1));
	}
	else
	{
		FATAL(L"Unsupported file path relation");
	}
}
