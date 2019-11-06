#include "pch.h"
#include "Solution.h"
#include <External/json.h>
#include <Filesystem/File.h>
#include <String/StringTemplate.h>

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
		}
	}

	Println(L"Writing to file %...", aFilePath);

	std::wofstream f(aFilePath.ToStdWString(), std::ios::binary);

	if (!f.is_open())
		FATAL(L"Failed to open file % for writing", aFilePath);

	f.write(aNewFileContents.GetData(), aNewFileContents.GetLength());
}

Solution::Solution(const StringView aSolutionDirectory)
{
	Println(L"Opening solution in directory %...", aSolutionDirectory);
	
	Println(L"Indexing files...");
	directory = MakeUnique<Directory>(nullptr, aSolutionDirectory);

	solutionName = String(directory->GetDirectoryName());

	Println(L"Opening solution %...", solutionName);

	if (const File* configFile = directory->GetFile(L"HeaderTool.json"))
	{
		const std::string fileContents = configFile->ReadString().View().ToStdString();

		json document = json::parse(fileContents);
		
		const String templateDirectoryName(document["templateDirectory"].get<std::string>().c_str());

		vsCppProjectTypeGuid = String(document["vsCppProjectTypeGuid"].get<std::string>().c_str());
		solutionGuid = String(document["solutionGuid"].get<std::string>().c_str());

		Directory* templateDirectory = directory->GetDirectory(templateDirectoryName);

		if (!templateDirectory)
			FATAL(L"Could not open template directory %", templateDirectoryName);

		File* solutionTemplateFile = templateDirectory->GetFile(L"SolutionTemplate.txt");
		if (!solutionTemplateFile)
			FATAL(L"Can't find SolutionTemplate.txt in template directory");
		String solutionTemplate = solutionTemplateFile->ReadString();

		File* libraryProjectTemplateFile = templateDirectory->GetFile(L"LibraryProjectTemplate.txt");
		if (!libraryProjectTemplateFile)
			FATAL(L"Can't find LibraryProjectTemplate.txt in template directory");
		const String libraryProjectTemplate = libraryProjectTemplateFile->ReadString();

		File* executableProjectTemplateFile = templateDirectory->GetFile(L"ExecutableProjectTemplate.txt");
		if (!executableProjectTemplateFile)
			FATAL(L"Can't find ExecutableProjectTemplate.txt in template directory");
		const String executableProjectTemplate = executableProjectTemplateFile->ReadString();

		Map<String, Project*> projectMap;

		for (const auto& projectDirectoryNameProperty : document["projects"].get<json::array_t>())
		{
			const std::string projectDirectoryStdName = projectDirectoryNameProperty.get<std::string>();
			const String projectDirectoryName(projectDirectoryStdName.c_str());

			if (Directory* projectDirectory = directory->GetDirectory(projectDirectoryName))
			{
				UniquePtr<Project> project = MakeUnique<Project>(projectDirectory);

				Project*& projectPtr = projectMap.FindOrAdd(project->projectName.View());

				if (projectPtr)
					FATAL(L"Project % is already defined.", project->projectName);

				projectPtr = project;

				projects.Add(Move(project));
			}
			else
			{
				FATAL(L"Project % does not appear to have a directory", projectDirectoryName);
			}
		}

		for (auto& project : projects)
			project->ResolveReferences(projectMap);

		GenerateSolutionFile(solutionTemplate);

		for (auto& project : projects)
		{
			StringView projectTemplate;
			if (project->projectType == ProjectType::Executable)
				projectTemplate = executableProjectTemplate;
			else if (project->projectType == ProjectType::Library)
				projectTemplate = libraryProjectTemplate;
			else
				FATAL(L"Unhandled project template %", static_cast<i32>(project->projectType));

			project->GenerateProjectFile(projectTemplate);
		}
	}
	else
	{
		FATAL(L"Solution % does not contain a HeaderTool.json file", solutionName);
	}
}

void Solution::GenerateSolutionFile(const StringView aSolutionTemplate) const
{
	const String outputFile = Format(L"%/%.sln", directory->GetAbsolutePath(), solutionName);

	StringTemplate documentTemplate = StringTemplate(String(aSolutionTemplate));
	StringTemplate projectTemplate(String(L"Project(\"${ProjectTypeGuid}\") = \"${ProjectName}\", \"${ProjectName}\\${ProjectName}_generated.vcxproj\", \"${ProjectGuid}\""));
	StringTemplate configurationTemplate(String(L"\t\t${ProjectGuid}.Debug|x64.ActiveCfg = Debug|x64\n\t\t${ProjectGuid}.Debug|x64.Build.0 = Debug|x64\n\t\t${ProjectGuid}.Release|x64.ActiveCfg = Release|x64\n\t\t${ProjectGuid}.Release|x64.Build.0 = Release|x64"));

	CHECK(vsCppProjectTypeGuid.GetLength() > 0);

	{
		String projectSection;
		String configurationSection;

		for (const UniquePtr<Project>& project : projects)
		{
			CHECK(project->projectName.GetLength() > 0);
			CHECK(project->projectGuid.GetLength() > 0);

			projectTemplate.AddParameter(String(L"ProjectTypeGuid"), String(vsCppProjectTypeGuid));
			projectTemplate.AddParameter(String(L"ProjectName"), String(project->projectName));
			projectTemplate.AddParameter(String(L"ProjectGuid"), String(project->projectGuid));

			projectSection.Append(projectTemplate.Evaluate().View());
			projectSection.Append(L"\nEndProject\n");

			projectTemplate.ClearParameters();

			configurationTemplate.AddParameter(String(L"ProjectGuid"), String(project->projectGuid));
			configurationSection.Append(configurationTemplate.Evaluate().View());
			configurationSection.Add(L'\n');
		
			configurationTemplate.ClearParameters();
		}

		if (configurationSection.TryGet(configurationSection.GetLength() - 1, L'\0') == L'\n')
			configurationSection.Pop();

		if (projectSection.TryGet(projectSection.GetLength() - 1, L'\0') == L'\n')
			projectSection.Pop();

		documentTemplate.AddParameter(String(L"ProjectSection"), Move(projectSection));
		documentTemplate.AddParameter(String(L"ConfigurationSection"), Move(configurationSection));
	}

	documentTemplate.AddParameter(String(L"SolutionGuid"), String(solutionGuid));

	const String output = documentTemplate.Evaluate();

	WriteToFileIfChanged(outputFile, output.View());
}
