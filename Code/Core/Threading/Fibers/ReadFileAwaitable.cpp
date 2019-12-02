#include "CorePch.h"
#include "ReadFileAwaitable.h"

ReadFileAwaitable::ReadFileAwaitable(const StringView& aPath)
{
	myPath = String(aPath);
}

Array<u8> ReadFileAwaitable::Work()
{
	std::wstring filePath = myPath.View().ToStdWString();
	std::ifstream fileStream(filePath);

	fmt::format(FMT_STRING(L"test {}"), myPath);
	
	if (!fileStream.good())
		FATAL(L"Failed to open file \"{}\"", myPath);

	fileStream.seekg(0, std::ios::end);
	Array<u8> data;
	data.Resize(static_cast<i32>(fileStream.tellg()));
	fileStream.seekg(0, std::ios::beg);
	fileStream.read(reinterpret_cast<char*>(data.GetData()), data.GetLength());

	// Println(L"Read % bytes from ", data.GetLength(), myPath);

	return data;
}
