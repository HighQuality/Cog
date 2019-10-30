#include "pch.h"
#include <External/json.h>
#include <fstream>
#include <Filesystem/Directory.h>
#include <Filesystem/File.h>
#include <String/GroupingWordReader.h>

int wmain(int argc, wchar_t* argv[], wchar_t* envp[])
{
	if (argc < 2)
	{
		Println(L"Too few inputs!");
		return 1;
	}
	
	String projectDirectory(argv[1]);

	for (Char& character : projectDirectory)
	{
		if (character == L'\\')
			character = L'/';
	}

	if (projectDirectory.ClampedSliceFromEnd(1) == L"/")
		projectDirectory.Pop();

	// std::ifstream f(projectDirectory.GetData());
	// if (!f.is_open())
	// {
	// 	Println(L"Failed to open configuration file %", StringView(argv[1]));
	// 	return 2;
	// }
	// 
	// nlohmann::json document;
	// f >> document;
	// 
	// Println(L"", document["type-list"].get<std::string>().c_str());

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

						const StringView ClassType = reader.GetCurrentWordOrGroup();

						if (ClassType == L"struct" || ClassType == L"class")
						{
							if (reader.Next() && !reader.IsAtGroup())
							{
								const StringView ClassName = reader.GetCurrentWordOrGroup();

								Println(L"COGTYPE % declared with %", ClassName, ClassType);
							}
							else
							{
								Println(L"Expected class name");
								return 4;
							}
						}
						else
						{
							Println(L"Expected \"struct\" or \"class\"");
							return 3;
						}
					}
				}
			}
		}
	}


	std::cin.get();
	return 0;
}
