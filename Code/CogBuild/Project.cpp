#include "CogBuildPch.h"
#include "Project.h"
#include <External\json.h>
#include <Filesystem/Directory.h>
#include <Filesystem/File.h>
#include <String/StringTemplate.h>
#include "CogBuildUtilities.h"
#include <String/GroupingWordReader.h>

Project::Project(Directory* aProjectDirectory)
{
	CHECK(aProjectDirectory);

	directory = aProjectDirectory;
	projectName = aProjectDirectory->GetName();
	
	extraIncludePaths.Add(Format(L"$(SolutionDir)%\\", projectName));

	buildProjectFile = Format(L"%/%.vcxproj", aProjectDirectory->GetAbsolutePath(), projectName);

	Println(L"Opening project %...", projectName);

	if (const File* configFile = directory->GetFile(Format(L"%.json", projectName).View()))
	{
		const std::string fileContents = configFile->ReadString().View().ToStdString();

		json document = json::parse(fileContents);

		if (document.find("references") != document.end())
		{
			const auto& referencesArray = document["references"];
			CHECK(referencesArray.is_array());

			for (const auto& reference : referencesArray.get<json::array_t>())
				namesOfReferences.Add(String(reference.get<std::string>().c_str()));
		}

		if (document.find("includePaths") != document.end())
		{
			const auto& includePathsArray = document["includePaths"];
			CHECK(includePathsArray.is_array());

			for (const auto& includePath : includePathsArray.get<json::array_t>())
				extraIncludePaths.Add(String(includePath.get<std::string>().c_str()));
		}

		if (document.find("linkDependencies") != document.end())
		{
			const auto& linkDependenciesArray = document["linkDependencies"];
			CHECK(linkDependenciesArray.is_array());

			for (const auto& linkDependency : linkDependenciesArray.get<json::array_t>())
				linkDependencies.Add(String(linkDependency.get<std::string>().c_str()));
		}

		if (document.find("libraryPaths") != document.end())
		{
			const auto& libraryPathsArray = document["libraryPaths"];
			CHECK(libraryPathsArray.is_array());

			for (const auto& libraryPath : libraryPathsArray.get<json::array_t>())
				extraLibraryPaths.Add(String(libraryPath.get<std::string>().c_str()));
		}

		preprocess = document["preprocess"].get<bool>();
		projectGuid = String(document["projectGuid"].get<std::string>().c_str());

		const std::string typeName = document["type"].get<std::string>();

		if (typeName == "executable")
		{
			projectType = ProjectType::Executable;
			debugDevelopmentProjectFile = Format(L"%/Debug%.vcxproj", aProjectDirectory->GetAbsolutePath(), projectName);
			debugDevelopmentUserProjectFile = Format(L"%/Debug%.vcxproj.user", aProjectDirectory->GetAbsolutePath(), projectName);
		}
		else if (typeName == "library")
			projectType = ProjectType::Library;
		else
			FATAL(L"Unknown project type % in project %", typeName.c_str(), projectName);

		directory->IterateFiles([&](const File& aFile)
			{
				if (aFile.GetExtension() == L".h" || aFile.GetExtension() == L".hpp")
					headerFiles.Add(&aFile);
				else if (aFile.GetExtension() == L".c" || aFile.GetExtension() == L".cpp" || aFile.GetExtension() == L".cc")
					sourceFiles.Add(&aFile);
			}, true);
	}
	else
	{
		FATAL(L"Project % does not contain a %.json file", projectName, projectName);
	}
}

void Project::ResolveReferences(const Map<String, Project*>& aProjects)
{
	for (const auto& referenceName : namesOfReferences)
	{
		if (Project* referencedProject = aProjects.Find(referenceName.View(), nullptr))
		{
			references.Add(referencedProject);
			extraIncludePaths.Add(Format(L"$(SolutionDir)%\\", referencedProject->projectName));
			linkDependencies.Add(Format(L"%.lib", referencedProject->projectName));
		}
		else
		{
			FATAL(L"Can't find referenced project % in project %", referenceName, projectName);
		}
	}
}

void Project::GenerateBuildProjectFile(StringView aProjectTemplate) const
{
	CHECK(buildProjectFile.GetLength() > 0);

	StringTemplate documentTemplate = StringTemplate(String(aProjectTemplate));
	CHECK(projectGuid.GetLength() > 0);
	documentTemplate.AddParameter(String(L"ProjectGuid"), String(projectGuid));
	CHECK(projectName.GetLength() > 0);
	documentTemplate.AddParameter(String(L"ProjectName"), String(projectName));

	{
		Map<StringView, u8> includePathsMap;
		GatherIncludePaths(includePathsMap);

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
		Map<StringView, u8> libraryPathsMap;
		GatherLibraryPaths(libraryPathsMap);

		String libraryPaths;
		for (const auto& pair : libraryPathsMap)
		{
			libraryPaths.Append(pair.key);
			libraryPaths.Add(L';');
		}

		libraryPaths.Replace(L'/', L'\\');

		documentTemplate.AddParameter(String(L"ExtraLibraryPaths"), Move(libraryPaths));
	}

	{
		Map<StringView, u8> linkDependenciesMap;
		GatherLinkDependencies(linkDependenciesMap);

		String extraLinkDependencies;
		for (const auto& pair : linkDependenciesMap)
		{
			extraLinkDependencies.Append(pair.key);
			extraLinkDependencies.Add(L';');
		}

		extraLinkDependencies.Replace(L'/', L'\\');

		documentTemplate.AddParameter(String(L"ExtraLinkDependencies"), Move(extraLinkDependencies));
	}

	{
		Map<Project*, u8> projectReferencesMap;
		GatherProjectReferences(projectReferencesMap);

		String projectReferences;

		for (const auto& pair : projectReferencesMap)
		{
			projectReferences.Append(Format(L"    <ProjectReference Include=\"..\\%\\%.vcxproj\">\n", pair.key->projectName, pair.key->projectName).View());
			projectReferences.Append(Format(L"      <Project>%</Project>\n", pair.key->projectGuid).View());
			projectReferences.Append(L"    </ProjectReference>\n");
		}

		if (projectReferences.GetLength() > 0)
			projectReferences.Pop();

		documentTemplate.AddParameter(String(L"ProjectReferences"), Move(projectReferences));
	}

	{
		String sourceFileList;

		const String pchFileName = Format(L"%Pch.cpp", projectName);

		for (const File* sourceFile : sourceFiles)
		{
			if (sourceFile->GetFilename() == pchFileName.View())
				continue;

			String sourceFilePath(sourceFile->GetRelativePath(*directory));
			sourceFilePath.Replace(L'/', L'\\');
				
			sourceFileList.Append(L"    <ClCompile Include=\"");
			sourceFileList.Append(sourceFilePath);
			sourceFileList.Append(L"\" />\n");
		}

		// Pop last \n
		if (sourceFileList.GetLength() > 0)
			sourceFileList.Pop();

		documentTemplate.AddParameter(String(L"SourceFiles"), Move(sourceFileList));
	}

	{
		String headerFileList;

		const String pchFileName = Format(L"%Pch.h", projectName);

		for (const File* headerFile : headerFiles)
		{
			if (headerFile->GetFilename() == pchFileName.View())
				continue;

			String headerFilePath(headerFile->GetRelativePath(*directory));
			headerFilePath.Replace(L'/', L'\\');

			headerFileList.Append(L"    <ClInclude Include=\"");
			headerFileList.Append(headerFilePath);
			headerFileList.Append(L"\" />\n");
		}

		// Pop last \n
		if (headerFileList.GetLength() > 0)
			headerFileList.Pop();

		documentTemplate.AddParameter(String(L"HeaderFiles"), Move(headerFileList));
	}

	const String output = documentTemplate.Evaluate();

	WriteToFileIfChanged(buildProjectFile, output.View());
}

void Project::GenerateDebugDevelopmentProjectFile(const StringView aMainProjectFilePath, const StringView aMainProjectGuid, const StringView aProjectTemplate, const StringView nmakeDebugUserFileTemplate) const
{
	CHECK(projectType == ProjectType::Executable);
	CHECK(debugDevelopmentProjectFile.GetLength() > 0);

	StringTemplate documentTemplate = StringTemplate(String(aProjectTemplate));
	CHECK(projectGuid.GetLength() > 0);
	documentTemplate.AddParameter(String(L"ProjectGuid"), String(projectGuid));

	documentTemplate.AddParameter(String(L"ExtraIncludePaths"), String());

	documentTemplate.AddParameter(String(L"SourceFiles"), String());
	documentTemplate.AddParameter(String(L"HeaderFiles"), String());
	documentTemplate.AddParameter(String(L"BuildCommandLine"), String());
	documentTemplate.AddParameter(String(L"RebuildCommandLine"), String());
	documentTemplate.AddParameter(String(L"CleanCommandLine"), String());
	documentTemplate.AddParameter(String(L"OutputFile"), Format(L"$(SolutionDir)..\\bin\\%_$(Configuration)_$(Platform).exe", projectName));

	documentTemplate.AddParameter(String(L"ProjectReferences"), Format(L"\
  <ItemGroup>\n\
    <ProjectReference Include=\"%\">\n\
      <Project>%</Project>\n\
    </ProjectReference>\n\
  </ItemGroup>\n", aMainProjectFilePath, aMainProjectGuid));

	const String output = documentTemplate.Evaluate();

	WriteToFileIfChanged(debugDevelopmentProjectFile, output.View());

	WriteToFileIfChanged(debugDevelopmentUserProjectFile, nmakeDebugUserFileTemplate);
}

void Project::GenerateCode(const StringView aGeneratedCodeDirectory)
{
	const String pchFileName = Format(L"%Pch.h", projectName);

	for (const File* file : headerFiles)
	{
		if (file->GetName() == pchFileName.View())
			continue;

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
							ReportErrorInFile(L"Expected group", file->GetAbsolutePath(), reader.CalculateAndGetCurrentLineIndex(), reader.CalculateAndGetCurrentColumnIndex());
							return;
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
								ReportErrorInFile(L"Expected class name", file->GetAbsolutePath(), reader.CalculateAndGetCurrentLineIndex(), reader.CalculateAndGetCurrentColumnIndex());
								return;
							}
						}
						else
						{
							ReportErrorInFile(Format(L"Expected \"struct\" or \"class\", got %", classType).View(), file->GetAbsolutePath(), reader.CalculateAndGetCurrentLineIndex(), reader.CalculateAndGetCurrentColumnIndex());
							return;
						}
					}
				}
				else if (reader.GetCurrentWordOrGroup() == L"�")
				{
					ReportErrorInFile(L"Detected �", file->GetAbsolutePath(), reader.CalculateAndGetCurrentLineIndex(), reader.CalculateAndGetCurrentColumnIndex());
				}
			}
		}
	}
}

void Project::GatherLibraryPaths(Map<StringView, u8>& aLibraryPaths) const
{
	for (const String& libraryPath : extraLibraryPaths)
		aLibraryPaths.FindOrAdd(libraryPath.View()) = 0;

	for (Project* reference : references)
		reference->GatherLibraryPaths(aLibraryPaths);
}

void Project::GatherLinkDependencies(Map<StringView, u8>& aLinkDependencies) const
{
	for (const String& linkDependency : linkDependencies)
		aLinkDependencies.FindOrAdd(linkDependency.View()) = 0;

	for (Project* reference : references)
		reference->GatherLinkDependencies(aLinkDependencies);
}

void Project::GatherIncludePaths(Map<StringView, u8>& aIncludePaths) const
{
	for (const String& includePath : extraIncludePaths)
		aIncludePaths.FindOrAdd(includePath.View()) = 0;

	for (Project* reference : references)
		reference->GatherIncludePaths(aIncludePaths);
}

void Project::GatherProjectReferences(Map<Project*, u8>& aProjectReferences) const
{
	for (Project* referencedProject : references)
		aProjectReferences.FindOrAdd(referencedProject) = 0;

	for (Project* reference : references)
		reference->GatherProjectReferences(aProjectReferences);
}

void Project::GatherHeaderFiles(Map<const File*, u8>& aHeaderFiles) const
{
	for (const File* headerFile : headerFiles)
		aHeaderFiles.FindOrAdd(headerFile) = 0;

	for (Project* reference : references)
		reference->GatherHeaderFiles(aHeaderFiles);
}

void Project::GatherSourceFiles(Map<const File*, u8>& aSourceFiles) const
{
	for (const File* sourceFile : sourceFiles)
		aSourceFiles.FindOrAdd(sourceFile) = 0;

	for (Project* reference : references)
		reference->GatherSourceFiles(aSourceFiles);
}