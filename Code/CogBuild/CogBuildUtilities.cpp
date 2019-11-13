#include "CogBuildPch.h"
#include "CogBuildUtilities.h"

void WriteToFileIfChanged(const StringView aFilePath, const StringView aNewFileContents)
{
	{
		std::wifstream readStream(aFilePath.ToStdWString(), std::ios::binary);

		if (readStream.is_open())
		{
			readStream.seekg(0, std::ios::end);
			String existingDocument;
			existingDocument.Resize(static_cast<i32>(readStream.tellg()));
			readStream.seekg(0, std::ios::beg);
			readStream.read(existingDocument.GetData(), existingDocument.GetLength());

			if (existingDocument == aNewFileContents)
				return;

			Println(L"Writing to file %...", aFilePath);
		}
	}

	std::wofstream f(aFilePath.ToStdWString(), std::ios::binary);

	if (!f.is_open())
		FATAL(L"Failed to open file % for writing", aFilePath);

	f.write(aNewFileContents.GetData(), aNewFileContents.GetLength());
}

void ReportErrorInFile(const StringView aMessage, const StringView aFilePath, const i32 aRowIndex /* = -1*/, const i32 aColumnIndex /* = -1*/)
{
	const String message = Format(L"error: %", aMessage);
	ReportMessageInFile(message.View(), aFilePath, aRowIndex, aColumnIndex);
	abort();
}

void ReportWarningInFile(const StringView aMessage, const StringView aFilePath, const i32 aRowIndex /* = -1*/, const i32 aColumnIndex /* = -1*/)
{
	ReportMessageInFile(Format(L"warning: %", aMessage).View(), aFilePath, aRowIndex, aColumnIndex);
}

void ReportMessageInFile(const StringView aMessage, const StringView aFilePath, const i32 aRowIndex /* = -1*/, const i32 aColumnIndex /* = -1*/)
{
	if (aRowIndex == -1)
		Println(L"%: %", aFilePath, aMessage);
	else if (aColumnIndex == -1)
		Println(L"%(%): %", aFilePath, aRowIndex + 1, aMessage);
	else
		Println(L"%(%,%): %", aFilePath, aRowIndex + 1, aColumnIndex + 1, aMessage);

	std::wcout.flush();
}
