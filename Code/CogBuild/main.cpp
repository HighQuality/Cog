#include "CogBuildPch.h"
#include <Solution.h>
#include <Time\Stopwatch.h>

int wmain(int argc, wchar_t* argv[], wchar_t* envp[])
{
	/*GroupingWordReader reader(L"\
{\n\
// {\n\
// }\n\
}");

	while (reader.Next())
	{
		if (reader.IsAtGroup())
			Println(L" \"%\"", reader.GetOpeningSequence());

		Println(L"% \"%\"", reader.IsAtGroup() ? L"\t" : L"", reader.GetCurrentWordOrGroup());

		if (reader.IsAtGroup())
		{
			Println(L" \"%\"", reader.GetClosingSequence());
			Println(L"---");
		}
	}
	
	std::cin.get();*/

	defer { if (IsDebuggerPresent()) std::cin.get(); };

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
	projectDirectory.Replace(L'/', L'\\');

	projectDirectory = FileSystemEntry::EvaluatePath(Move(projectDirectory));

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
	
	// Generate code before generating the build project files so the generated code can be included
	if (buildProject)
	{
		Println(L"Generating code...");
		Stopwatch w;
		
		if (!solution.GenerateCode())
		{
			Println(L"Code generation failed");
			return 1;
		}

		Println(L"Done in %ms", w.GetElapsedTime().Milliseconds());
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

	return 0;
}
