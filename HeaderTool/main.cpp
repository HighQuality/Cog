#include "pch.h"
#include <fstream>
#include <Filesystem/Directory.h>
#include <Filesystem/File.h>
#include <String/GroupingWordReader.h>
#include <Solution.h>

int wmain(int argc, wchar_t* argv[], wchar_t* envp[])
{
	if (argc < 2)
	{
		Println(L"Too few inputs!");
		return 1;
	}
	
	String projectDirectory(argv[1]);
	projectDirectory.Replace(L'\\', L'/');

	if (projectDirectory.ClampedSliceFromEnd(1) == L"/")
		projectDirectory.Pop();
	
	Solution solution(projectDirectory);

	// std::ofstream typeIncludeFileStream(Format(L"%/%", projectDirectoryStd, typeIncludeFile);
	// typeIncludeFileStream << "#pragma once" << std::endl;
	// 
	// CHECK(typeIncludeFileStream.is_open());

	Directory d(nullptr, projectDirectory);

	Array<const File*> headerFiles;

	d.IterateFiles([&](const File& aFile)
		{
			const StringView extension = aFile.GetExtension();

			StringView filenameWithoutExtension = aFile.GetFilenameWithoutExtension();

			// Skip precompiled headers
			if (filenameWithoutExtension == L"stdafx" ||
				filenameWithoutExtension == L"pch" ||
				filenameWithoutExtension == L"exported_pch")
			{
				return;
			}

			if (extension == L".h" || extension == L".hpp")
			{
				headerFiles.Add(&aFile);
			}
		}, true);

	for (const File* file : headerFiles)
	{
		const String fileContents = file->ReadString();

		GroupingWordReader reader(fileContents); 

		while (reader.Next())
		{
			if (!reader.IsAtGroup())
			{
				if (reader.GetCurrentWordOrGroup() == L"COGTYPE")
				{
					Println(L"", file->GetFilename());

					if (reader.Next())
					{
						if (!reader.IsAtGroup() || reader.GetOpeningCharacter() != L'(')
						{
							Println(L"Expected group");
							return 5;
						}

						GroupingWordReader parameterReader(reader.GetCurrentGroup());

						while (parameterReader.Next())
						{
							Println(L"\t", parameterReader.GetCurrentWordOrGroup());
						}

						reader.Next();

						const StringView classType = reader.GetCurrentWordOrGroup();

						if (classType == L"struct" || classType == L"class")
						{
							if (reader.Next() && !reader.IsAtGroup())
							{
								const StringView className = reader.GetCurrentWordOrGroup();

								// typeIncludeFileStream << std::string(Format(L"#include \"%\"", file->GetAbsolutePath())) << std::endl;
								
								Println(L"COGTYPE % declared with %", className, classType);
							}
							else
							{
								Println(L"Expected class name");
								return 4;
							}
						}
						else
						{
							Println(L"Expected \"struct\" or \"class\", got %", classType);
							return 3;
						}
					}
				}
			}
		}
	}
	
	return 0;
}
