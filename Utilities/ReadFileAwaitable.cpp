#include "pch.h"
#include "ReadFileAwaitable.h"

ReadFileAwaitable::ReadFileAwaitable(const StringView& aPath)
{
	myPath = String(aPath);
}

void ReadFileAwaitable::SynchronousWork()
{
	std::wstring filePath = myPath.View().ToStdWString();
	std::ifstream fileStream(filePath);

	if (!fileStream.good())
		FATAL(L"Failed to open file \"%\"", myPath);

	fileStream.seekg(0, std::ios::end);
	Array<u8> data;
	data.Resize(static_cast<i32>(fileStream.tellg()));
	fileStream.seekg(0, std::ios::beg);
	fileStream.read(reinterpret_cast<char*>(data.GetData()), data.GetLength());

	Println(L"Read % bytes from ", data.GetLength(), myPath);
}
