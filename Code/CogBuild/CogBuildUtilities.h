#pragma once

void WriteToFileIfChanged(StringView aFilePath, StringView aNewFileContents);
void ReportErrorInFile(StringView aMessage, StringView aFilePath, i32 aRowIndex = -1, i32 aColumnIndex = -1);
void ReportWarningInFile(StringView aMessage, StringView aFilePath, i32 aRowIndex = -1, i32 aColumnIndex = -1);
void ReportMessageInFile(StringView aMessage, StringView aFilePath, i32 aRowIndex = -1, i32 aColumnIndex = -1);
