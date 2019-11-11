#include "CogBuildPch.h"
#include "Solution.h"
#include <External/json.h>
#include <Filesystem/File.h>
#include <String/StringTemplate.h>
#include "CogBuildUtilities.h"

Solution::Solution(const StringView aSolutionDirectory)
{
	Println(L"Opening solution in directory %...", aSolutionDirectory);
	
	Println(L"Indexing files...");
	directory = MakeUnique<Directory>(nullptr, aSolutionDirectory);

	codeDirectory = directory->GetDirectory(L"Code");
	CHECK_MSG(codeDirectory, L"Couldn't find directory \"Code\" inside of solution");

	solutionName = String(directory->GetDirectoryName());
	buildSolutionFile = Format(L"%/Code/Build%.sln", directory->GetAbsolutePath(), solutionName);
	developmentSolutionFile = Format(L"%/Code/%.sln", directory->GetAbsolutePath(), solutionName);
	developmentMainProjectName = Format(L"Develop%", solutionName);
	developmentMainProjectFile = Format(L"%/Code/%.vcxproj", directory->GetAbsolutePath(), developmentMainProjectName);

	Println(L"Opening solution %...", solutionName);

	if (const File* configFile = directory->GetFile(L"CogBuildConfig.json"))
	{
		const std::string fileContents = configFile->ReadString().View().ToStdString();

		json document = json::parse(fileContents);
		
		vsCppProjectTypeGuid = String(document["vsCppProjectTypeGuid"].get<std::string>().c_str());
		solutionGuid = String(document["solutionGuid"].get<std::string>().c_str());
		developmentMainProjectGuid = String(document["developmentProjectGuid"].get<std::string>().c_str());

		const String templateDirectoryPath(document["templateDirectory"].get<std::string>().c_str());
		
		Directory* templateDirectory = directory->GetDirectory(templateDirectoryPath);

		if (!templateDirectory)
			FATAL(L"Could not open template directory %", templateDirectoryPath);

		templates.ReadFromDirectory(*templateDirectory);

		Map<String, Project*> projectMap;

		for (const auto& projectDirectoryNameProperty : document["projects"].get<json::array_t>())
		{
			const std::string projectDirectoryStdName = projectDirectoryNameProperty.get<std::string>();
			const String projectDirectoryName = Format(L"Code/%", projectDirectoryStdName.c_str());

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
	}
	else
	{
		FATAL(L"Solution % does not contain a CogBuildConfig.json file", solutionName);
	}
}

void Solution::ReadTemplates(const StringView aTemplateDirectory)
{

}

void Solution::GenerateBuildProjects() const
{
	Array<SolutionDocumentProjectReference> solutionProjects;

	for (auto& project : projects)
	{
		StringView projectTemplate;
		if (project->projectType == ProjectType::Executable)
			projectTemplate = templates.executableProjectTemplate;
		else if (project->projectType == ProjectType::Library)
			projectTemplate = templates.libraryProjectTemplate;
		else
			FATAL(L"Unhandled project template %", static_cast<i32>(project->projectType));

		solutionProjects.Add(SolutionDocumentProjectReference(project->projectGuid, project->projectName, project->buildProjectFile));

		project->GenerateBuildProjectFile(projectTemplate);
	}
	
	GenerateSolutionFile(buildSolutionFile, solutionProjects);
}

void Solution::GenerateDevelopmentProjects(const StringView aBuildToolPath) const
{
	Array<SolutionDocumentProjectReference> solutionProjects;

	for (auto& project : projects)
	{
		if (project->projectType == ProjectType::Executable)
		{
			solutionProjects.Add(SolutionDocumentProjectReference(project->projectGuid, project->projectName, project->debugDevelopmentProjectFile));

			project->GenerateDebugDevelopmentProjectFile(developmentMainProjectFile, developmentMainProjectGuid, templates);
		}
	}

	GenerateDevelopmentMainProjectFile(aBuildToolPath);
	solutionProjects.Add(SolutionDocumentProjectReference(developmentMainProjectGuid, developmentMainProjectName, developmentMainProjectFile));

	GenerateSolutionFile(developmentSolutionFile, solutionProjects);
}

void Solution::GenerateDevelopmentMainProjectFile(const StringView aBuildToolPath) const
{
	StringTemplate documentTemplate = StringTemplate(String(templates.developmentProjectTemplate));

	documentTemplate.AddParameter(String(L"ProjectGuid"), String(developmentMainProjectGuid));
	documentTemplate.AddParameter(String(L"ProjectReferences"), String());
	documentTemplate.AddParameter(String(L"OutputFile"), String());

	documentTemplate.AddParameter(String(L"BuildCommandLine"), Format(L"\"%\" -Build % $(Configuration) $(Platform)", aBuildToolPath, directory->GetAbsolutePath()));
	documentTemplate.AddParameter(String(L"RebuildCommandLine"), Format(L"\"%\" -Rebuild % $(Configuration) $(Platform)", aBuildToolPath, directory->GetAbsolutePath()));
	documentTemplate.AddParameter(String(L"CleanCommandLine"), Format(L"\"%\" -Clean %", aBuildToolPath, directory->GetAbsolutePath()));

	{
		Map<StringView, u8> includePathsMap;
		for (const auto& project : projects)
			project->GatherIncludePaths(includePathsMap);

		String includePaths;
		
		for (const auto& pair : includePathsMap)
		{
			includePaths.Append(pair.key);
			includePaths.Add(L';');
		}

		includePaths.Replace(L'/', L'\\');

		documentTemplate.AddParameter(String(L"ExtraIncludePaths"), Move(includePaths));
	}

	{
		Map<const File*, u8> sourceFiles;
		for (const auto& project : projects)
			project->GatherSourceFiles(sourceFiles);

		String sourceFileList;

		if (sourceFiles.GetLength() > 0)
		{
			for (const auto& sourceFilePair : sourceFiles)
			{
				String sourceFile(sourceFilePair.key->GetRelativePath(*codeDirectory));
				sourceFile.Replace(L'/', L'\\');

				sourceFileList.Append(L"    <ClCompile Include=\"");
				sourceFileList.Append(sourceFile);
				sourceFileList.Append(L"\" />\n");
			}

			// Pop last \n
			sourceFileList.Pop();
		}

		documentTemplate.AddParameter(String(L"SourceFiles"), Move(sourceFileList));
	}

	{
		Map<const File*, u8> headerFiles;
		for (const auto& project : projects)
			project->GatherHeaderFiles(headerFiles);

		String headerFileList;

		if (headerFiles.GetLength() > 0)
		{
			for (const auto& headerFilePair : headerFiles)
			{
				String headerFile(headerFilePair.key->GetRelativePath(*codeDirectory));
				headerFile.Replace(L'/', L'\\');

				headerFileList.Append(L"    <ClInclude Include=\"");
				headerFileList.Append(headerFile);
				headerFileList.Append(L"\" />\n");
			}

			// Pop last \n
			headerFileList.Pop();
		}

		documentTemplate.AddParameter(String(L"HeaderFiles"), Move(headerFileList));
	}

	const String output = documentTemplate.Evaluate();

	WriteToFileIfChanged(developmentMainProjectFile, output.View());
}

bool Solution::GenerateCode()
{
	CreateDirectoryW(Format(L"%/temp", directory->GetAbsolutePath()).GetData(), nullptr);

	Array<Project*> preprocessProjects;

	for (const auto& project : projects)
	{
		if (!project->preprocess)
			continue;

		preprocessProjects.Add(project);
	}

	for (Project* project : preprocessProjects)
	{
		/*
		const String projectGeneratedCodeDirectory = Format(L"%/temp/%", directory->GetAbsolutePath(), project->projectName);
		CreateDirectoryW(projectGeneratedCodeDirectory.GetData(), nullptr);
		*/

		if (!project->ParseHeaders())
			return false;
	}

	return true;
}

void Solution::GenerateSolutionFile(StringView aSolutionFilePath, ArrayView<SolutionDocumentProjectReference> aProjects) const
{
	templates.CheckIsLoaded();

	StringTemplate documentTemplate = StringTemplate(String(templates.solutionTemplate));
	StringTemplate projectTemplate(String(L"Project(\"${ProjectTypeGuid}\") = \"${ProjectName}\", \"${ProjectFilePath}\", \"${ProjectGuid}\""));
	StringTemplate configurationTemplate(String(L"\t\t${ProjectGuid}.Debug|x64.ActiveCfg = Debug|x64\n\t\t${ProjectGuid}.Debug|x64.Build.0 = Debug|x64\n\t\t${ProjectGuid}.Release|x64.ActiveCfg = Release|x64\n\t\t${ProjectGuid}.Release|x64.Build.0 = Release|x64"));

	CHECK(vsCppProjectTypeGuid.GetLength() > 0);

	{
		String projectSection;
		String configurationSection;

		for (const SolutionDocumentProjectReference& project : aProjects)
		{
			CHECK(project.projectName.GetLength() > 0);
			CHECK(project.guid.GetLength() > 0);
			CHECK(project.projectFilePath.GetLength() > 0);

			projectTemplate.AddParameter(String(L"ProjectTypeGuid"), String(vsCppProjectTypeGuid));
			projectTemplate.AddParameter(String(L"ProjectName"), String(project.projectName));
			projectTemplate.AddParameter(String(L"ProjectGuid"), String(project.guid));
			projectTemplate.AddParameter(String(L"ProjectFilePath"), String(project.projectFilePath));

			projectSection.Append(projectTemplate.Evaluate().View());
			projectSection.Append(L"\nEndProject\n");

			projectTemplate.ClearParameters();

			configurationTemplate.AddParameter(String(L"ProjectGuid"), String(project.guid));
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

	WriteToFileIfChanged(aSolutionFilePath, output.View());
}

