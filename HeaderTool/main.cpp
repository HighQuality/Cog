#include "pch.h"
#include <fstream>
#include <Filesystem/Directory.h>
#include <Filesystem/File.h>
#include <String/GroupingWordReader.h>
#include <Solution.h>

int wmain(int argc, wchar_t* argv[], wchar_t* envp[])
{
	// Println(L"% args", argc);
	// for (i32 i = 0; i < argc; ++i)
	// {
	// 	Println(L"\t", StringView(argv[i]));
	// }

	if (argc < 3)
	{
		Println(L"Usage: CogBuild -command directory");
		Println(L"Valid commands:");
		Println(L"\t-GenerateProjectFiles");
		Println(L"\t\tGenerate the visual studio project used by the build system to compile the application(s)");
		Println(L"\t-Build");
		Println(L"\t\tCompiles the solution in the directory generates project files if necessary");
		Println(L"\t-Clean");
		Println(L"\t\tCleans the project");
		Println(L"\t-Rebuild");
		Println(L"\t\tCleans and then builds the project");
		Println(L"\t-GenerateSolution");
		Println(L"\t\tGenerate the solution used to invoke the build system");
		return 1;
	}

	const String command(argv[1]);

	bool generateBuildProjects = false;
	bool generateDevelopmentProjects = false;
	
	bool buildProject = false;
	String configuration;
	String platform;

	bool cleanProject = false;

	if (command == L"-GenerateBuildProjects")
	{
		generateBuildProjects = true;
	}
	else if (command == L"-Build")
	{
		generateBuildProjects = true;
		buildProject = true;

		if (argc < 5)
		{
			Println(L"Usage:");
			Println(L"CogBuild -Build Directory Configuration Platform");
			return 1;
		}

		configuration = String(argv[3]);
		platform = String(argv[4]);
	}
	else if (command == L"-GenerateDevelopmentProjects")
	{
		generateDevelopmentProjects = true;
	}
	else if (command == L"-Clean")
	{
		cleanProject = true;
	}
	else if (command == L"-Rebuild")
	{
		cleanProject = true;
		generateBuildProjects = true;
		buildProject = true;

		if (argc < 5)
		{
			Println(L"Usage:");
			Println(L"CogBuild -Rebuild Directory Configuration Platform");
			return 1;
		}

		configuration = String(argv[3]);
		platform = String(argv[4]);
	}
	else
	{
		Println(L"Unknown command \"%\"", command);
		return 1;
	}

	String projectDirectory(argv[2]);
	projectDirectory.Replace(L'\\', L'/');

	if (projectDirectory.ClampedSliceFromEnd(1) == L"/")
		projectDirectory.Pop();

	Solution solution(projectDirectory);

	if (cleanProject)
	{
		String msBuildCleanCommand = Format(L"msbuild \"%\" -nologo -target:clean -maxcpucount -verbosity:minimal", solution.buildSolutionFile);

		std::wcout.flush();
		const i32 returnCode = system(msBuildCleanCommand.View().ToStdString().c_str());

		Println(L"msbuild clean returned %", returnCode);

	}
	
	if (generateBuildProjects)
	{
		solution.GenerateBuildProjects();
	}

	if (generateDevelopmentProjects)
	{
		solution.GenerateDevelopmentProjects(argv[0]);
	}

	if (buildProject)
	{
		CHECK(configuration.GetLength() > 0);
		CHECK(platform.GetLength() > 0);

		String buildCommand = Format(L"msbuild \"%\" -nologo -maxcpucount -verbosity:minimal \"/property:Configuration=%;Platform=%\"", solution.buildSolutionFile, configuration, platform);

		std::wcout.flush();
		const i32 returnCode = system(buildCommand.View().ToStdString().c_str());

		Println(L"msbuild returned %", returnCode);

		if (returnCode != 0)
			return returnCode;
	}

	Array<const File*> headerFiles;

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
					Println(L"", file->GetName());

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
