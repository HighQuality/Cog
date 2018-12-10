#include "pch.h"
#include "File.h"
#include <fstream>

File::File(Directory & aParentDirectory, const StringView & aAbsolutePath, const size_t aFileSize, const uint64_t aLastWriteTime)
	: FileSystemEntry(&aParentDirectory, aAbsolutePath)
{
	const StringView & Path = GetAbsolutePath();
	for (i32 i = Path.GetLength() - 1; i >= 0; --i)
	{
		if (Path[i] == L'\\' || Path[i] == L'/')
		{
			myFilename = Path.ChopFromStart(i + 1);
			break;
		}
	}

	if (myFilename.GetLength() == 0)
	{
		myFilename = Path;
	}

	for (i32 i = myFilename.GetLength() - 1; i >= 0; --i)
	{
		if (myFilename[i] == L'.')
		{
			myExtension = myFilename.ChopFromStart(i);
			break;
		}
	}

	myFilenameWithoutExtension = myFilename.ChopFromEnd(myExtension.GetLength());

	myFileSize = aFileSize;
	myLastWriteTime = aLastWriteTime;
}

File::~File()
{
}

bool File::IsDirectory() const
{
	return false;
}

size_t File::GetFileSize() const
{
	return myFileSize;
}

uint64_t File::GetLastWriteTime() const
{
	return myLastWriteTime;
}

Array<u8> File::ReadFile() const
{
	GetAbsolutePath().CheckEndsWithZero();
	std::ifstream fileStream(GetAbsolutePath().GetData());

	if (fileStream.good() == false)
		FATAL(L"File catalogued by the directory system is not readable");

	fileStream.seekg(0, std::ios::end);
	Array<u8> data;
	data.Resize(static_cast<i32>(fileStream.tellg()));
	fileStream.seekg(0, std::ios::beg);
	fileStream.read(reinterpret_cast<char*>(data.GetData()), data.GetLength());

	return data;
}

String File::ReadString() const
{
	GetAbsolutePath().CheckEndsWithZero();
	std::wifstream fileStream(GetAbsolutePath().GetData());

	if (fileStream.good() == false)
		FATAL(L"File catalogued by the directory system is not readable");

	fileStream.seekg(0, std::ios::end);
	String data;
	data.Resize(static_cast<i32>(fileStream.tellg()));
	fileStream.seekg(0, std::ios::beg);
	fileStream.read(data.GetData(), data.GetLength());

	return data;
}
