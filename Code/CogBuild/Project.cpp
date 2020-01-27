#include "CogBuildPch.h"
#include "Project.h"
#include <External\json.h>
#include <Filesystem/Directory.h>
#include <Filesystem/File.h>
#include <String/GroupingWordReader.h>
#include <String/StringTemplate.h>
#include "CogBuildUtilities.h"
#include "DocumentTemplates.h"
#include "CogClass.h"

Project::Project(Directory* aProjectDirectory)
{
	CHECK(aProjectDirectory);

	directory = aProjectDirectory;
	projectName = aProjectDirectory->GetName();
	generatedCodeDirectory = Format(L"%\\temp\\%\\generated", aProjectDirectory->GetParentDirectory()->GetParentDirectory()->GetAbsolutePath(), projectName);
	pchHeaderFileName = Format(L"%Pch.h", projectName);
	pchSourceFileName = Format(L"%Pch.cpp", projectName);

	extraIncludePaths.Add(Format(L"$(SolutionDir)%\\", projectName));
	extraIncludePaths.Add(Format(L"%\\", generatedCodeDirectory));

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
			extraIncludePaths.Add(Format(L"%\\", referencedProject->generatedCodeDirectory));
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
		Array<StringView> includePaths = GatherIncludePaths();

		String includePathsContent;
		for (const StringView path : includePaths)
		{
			includePathsContent.Append(path);
			includePathsContent.Add(L';');
		}

		includePathsContent.Replace(L'/', L'\\');

		documentTemplate.AddParameter(String(L"ExtraIncludePaths"), Move(includePathsContent));
	}

	{
		Array<StringView> libraryPaths = GatherLibraryPaths();

		String libraryPathsContent;
		for (const StringView path : libraryPaths)
		{
			libraryPathsContent.Append(path);
			libraryPathsContent.Add(L';');
		}

		libraryPathsContent.Replace(L'/', L'\\');

		documentTemplate.AddParameter(String(L"ExtraLibraryPaths"), Move(libraryPathsContent));
	}

	{
		Array<StringView> extraLinkDependencies = GatherLinkDependencies();

		String extraLinkDependenciesContent;
		for (const StringView dependency : extraLinkDependencies)
		{
			extraLinkDependenciesContent.Append(dependency);
			extraLinkDependenciesContent.Add(L';');
		}

		extraLinkDependenciesContent.Replace(L'/', L'\\');

		documentTemplate.AddParameter(String(L"ExtraLinkDependencies"), Move(extraLinkDependenciesContent));
	}

	{
		Array<Project*> projectReferences = GatherProjectReferences();

		String projectReferencesContent;

		for (const Project* project : projectReferences)
		{
			projectReferencesContent.Append(Format(L"    <ProjectReference Include=\"..\\%\\%.vcxproj\">\n", project->projectName, project->projectName).View());
			projectReferencesContent.Append(Format(L"      <Project>%</Project>\n", project->projectGuid).View());
			projectReferencesContent.Append(L"    </ProjectReference>\n");
		}

		if (projectReferencesContent.GetLength() > 0)
			projectReferencesContent.Pop();

		documentTemplate.AddParameter(String(L"ProjectReferences"), Move(projectReferencesContent));
	}

	{
		String sourceFileList;

		for (const File* sourceFile : sourceFiles)
		{
			if (sourceFile->GetFilename() == pchSourceFileName.View())
				continue;

			String sourceFilePath(sourceFile->GetRelativePath(*directory));
			sourceFilePath.Replace(L'/', L'\\');
				
			sourceFileList.Append(L"    <ClCompile Include=\"");
			sourceFileList.Append(sourceFilePath);
			sourceFileList.Append(L"\" />\n");
		}

		for (const String& sourceFile : generatedSourceFiles)
		{
			String sourceFilePath(sourceFile);
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

		for (const File* headerFile : headerFiles)
		{
			if (headerFile->GetFilename() == pchHeaderFileName.View())
				continue;

			String headerFilePath(headerFile->GetRelativePath(*directory));
			headerFilePath.Replace(L'/', L'\\');

			headerFileList.Append(L"    <ClInclude Include=\"");
			headerFileList.Append(headerFilePath);
			headerFileList.Append(L"\" />\n");
		}

		for (const String& generatedHeader : generatedHeaderFiles)
		{
			String headerFilePath(generatedHeader);
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

void Project::GenerateDebugDevelopmentProjectFile(const StringView aMainProjectFilePath, const StringView aMainProjectGuid, const DocumentTemplates& aTemplates) const
{
	CHECK(projectType == ProjectType::Executable);
	CHECK(debugDevelopmentProjectFile.GetLength() > 0);

	StringTemplate documentTemplate = StringTemplate(String(aTemplates.developmentProjectTemplate));
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

	WriteToFileIfChanged(debugDevelopmentUserProjectFile, aTemplates.nmakeDebugUserFileTemplate);
}

bool Project::ParseHeaders()
{
	for (const File* file : headerFiles)
	{
		if (file->GetName() == pchHeaderFileName.View())
			continue;

		String headerIncludePath = file->GetRelativePath(*directory);
		headerIncludePath.Replace(L'\\', L'/');

		HeaderParser& parser = *myHeaderParsers.Add(MakeUnique<HeaderParser>(file, String(headerIncludePath)));

		parser.Parse();

		if (parser.HasErrors())
		{
			for (const String& error : parser.GetErrors())
				Println(error);

			return false;
		}

		if (parser.HasGeneratedCode())
		{
			hasGeneratedAnyCode = true;

			const GeneratedCode& generatedCode = parser.GetGeneratedCode();

			generatedHeaderFiles.Add(Format(L"%/", generatedCodeDirectory, generatedCode.GetHeaderFileName()));
			generatedSourceFiles.Add(Format(L"%/", generatedCodeDirectory, generatedCode.GetSourceFileName()));

			typeListRegistratorFile = Format(L"%/%TypeListRegistrator.generated.cpp", generatedCodeDirectory, projectName);
			generatedSourceFiles.Add(typeListRegistratorFile);

			if (projectType == ProjectType::Executable)
			{
				typeListInvocatorFile = Format(L"%/%TypeListInvocator.generated.cpp", generatedCodeDirectory, projectName);
				generatedSourceFiles.Add(typeListInvocatorFile);
			}
		}
	}

	return true;
}

void Project::GenerateFiles(const DocumentTemplates& aTemplates) const
{
	if (!hasGeneratedAnyCode)
		return;

	CreateDirectoryW(generatedCodeDirectory.GetData(), nullptr);

	Array<CogClass*> cogClasses;
	Array<String> cogClassHeaderFiles;

	for (const HeaderParser* parser : myHeaderParsers)
	{
		const GeneratedCode& generatedCode = parser->GetGeneratedCode();
		
		if (parser->HasGeneratedCode())
		{
			generatedCode.WriteFiles(aTemplates, projectName, generatedCodeDirectory);

			cogClassHeaderFiles.Add(String(generatedCode.GetMainHeaderIncludePath()));

			for (CogClass* cogClass : generatedCode.GetCogClasses())
				cogClasses.Add(cogClass);
		}
	}

	{
		StringTemplate typeList(String(aTemplates.typeListTemplate));

		typeList.AddParameter(String(L"PchFileName"), String(pchHeaderFileName));
		typeList.AddParameter(String(L"ProjectName"), String(projectName));

		{
			String includes;

			for (StringView header : cogClassHeaderFiles)
			{
				includes.Append(Format(L"#include \"%\"\n", header).View());
			}

			typeList.AddParameter(String(L"Includes"), Move(includes));
		}

		{
			String typeListRegistrations;

			for (const CogClass* cogClass : cogClasses)
			{
				// Object is registered explicitly in the base TypeList object
				if (cogClass->GetTypeName() == L"Object")
					continue;

				if (cogClass->SpecializesBaseClass())
					typeListRegistrations.Append(Format(L"\tREGISTER_TYPE_SPECIALIZATION(aTypeList, %, %);\n", cogClass->GetBaseTypeName(), cogClass->GetTypeName()).View());
				else
					typeListRegistrations.Append(Format(L"\tREGISTER_TYPE(aTypeList, %);\n", cogClass->GetTypeName()).View());
			}

			typeList.AddParameter(String(L"TypeListRegistrations"), Move(typeListRegistrations));
		}

		const String typeListContent = typeList.Evaluate();

		WriteToFileIfChanged(typeListRegistratorFile.View(), typeListContent);
	}

	if (projectType == ProjectType::Executable)
	{
		StringTemplate typeListInvocator(String(aTemplates.typeListInvocatorTemplate));

		typeListInvocator.AddParameter(String(L"PchFileName"), String(pchHeaderFileName));

		{
			String declarations;
			String invocations;

			Array<const Project*> referencedProjects = Array<const Project*>(GatherProjectReferences());
			referencedProjects.Add(this);

			for (const Project* project : referencedProjects)
			{
				if (!project->preprocess)
					continue;

				declarations.Append(Format(L"void RegisterCogTypesForProject_%(TypeList* aTypeList);\n", project->projectName).View());
				invocations.Append(Format(L"\tRegisterCogTypesForProject_%(aTypeList);\n", project->projectName).View());
			}

			typeListInvocator.AddParameter(String(L"Declarations"), Move(declarations));
			typeListInvocator.AddParameter(String(L"Invocations"), Move(invocations));
		}

		const String typeListInvocatorContent = typeListInvocator.Evaluate();

		WriteToFileIfChanged(typeListInvocatorFile.View(), typeListInvocatorContent);
	}
}

bool Project::GatherCogTypes(Map<String, CogType*>& aCogTypes) const
{
	for (const HeaderParser* parser : myHeaderParsers)
	{
		for (CogType* cogType : parser->GetGeneratedCode().GetCogTypes())
		{
			CogType*& typeRegistration = aCogTypes.FindOrAdd(cogType->GetTypeName());

			if (typeRegistration)
			{
				const String theirLocation = typeRegistration->GetDeclarationLocation();
				const String ourLocation = cogType->GetDeclarationLocation();
				FATAL(L"There exists multiple COGTYPEs with the name %:\n%: error: First declaration\n%: error: Second declaration", typeRegistration->GetTypeName(), theirLocation, ourLocation);
				return false;
			}

			typeRegistration = cogType;
		}
	}

	return true;
}

bool Project::ResolveDependencies(const Map<String, CogType*>& aCogTypes)
{
	for (const HeaderParser* parser : myHeaderParsers)
	{
		for (CogType* cogType : parser->GetGeneratedCode().GetCogTypes())
		{
			if (!cogType->ResolveDependencies(aCogTypes))
				return false;
		}
	}
	return true;
}

void Project::GatherLibraryPathsMap(Map<StringView, u8>& aLibraryPaths) const
{
	for (const String& libraryPath : extraLibraryPaths)
		aLibraryPaths.FindOrAdd(libraryPath.View()) = 0;

	for (Project* reference : references)
		reference->GatherLibraryPathsMap(aLibraryPaths);
}

Array<StringView> Project::GatherLibraryPaths() const
{
	Map<StringView, u8> map;
	GatherLibraryPathsMap(map);
	
	Array<StringView> output = map.GetKeys();
	output.QuickSort();

	return output;
}

void Project::GatherLinkDependenciesMap(Map<StringView, u8>& aLinkDependencies) const
{
	for (const String& linkDependency : linkDependencies)
		aLinkDependencies.FindOrAdd(linkDependency.View()) = 0;

	for (Project* reference : references)
		reference->GatherLinkDependenciesMap(aLinkDependencies);
}

Array<StringView> Project::GatherLinkDependencies() const
{
	Map<StringView, u8> map;
	GatherLinkDependenciesMap(map);

	Array<StringView> output = map.GetKeys();
	output.QuickSort();

	return output;
}

void Project::GatherIncludePathsMap(Map<StringView, u8>& aIncludePaths) const
{
	for (const String& includePath : extraIncludePaths)
		aIncludePaths.FindOrAdd(includePath.View()) = 0;

	for (Project* reference : references)
		reference->GatherIncludePathsMap(aIncludePaths);
}

Array<StringView> Project::GatherIncludePaths() const
{
	Map<StringView, u8> map;
	GatherIncludePathsMap(map);

	Array<StringView> output = map.GetKeys();
	output.QuickSort();

	return output;
}

void Project::GatherProjectReferencesMap(Map<Project*, u8>& aProjectReferences) const
{
	for (Project* referencedProject : references)
		aProjectReferences.FindOrAdd(referencedProject) = 0;

	for (Project* reference : references)
		reference->GatherProjectReferencesMap(aProjectReferences);
}

Array<Project*> Project::GatherProjectReferences() const
{
	Map<Project*, u8> map;
	GatherProjectReferencesMap(map);

	Array<Project*> output = map.GetKeys();
	output.QuickSort([](Project* aA, Project* aB) { return aA->projectName < aB->projectName; });

	return output;
}

void Project::GatherHeaderFilesMap(Map<const File*, u8>& aHeaderFiles) const
{
	for (const File* headerFile : headerFiles)
		aHeaderFiles.FindOrAdd(headerFile) = 0;

	for (Project* reference : references)
		reference->GatherHeaderFilesMap(aHeaderFiles);
}

void Project::GatherSourceFilesMap(Map<const File*, u8>& aSourceFiles) const
{
	for (const File* sourceFile : sourceFiles)
		aSourceFiles.FindOrAdd(sourceFile) = 0;

	for (Project* reference : references)
		reference->GatherSourceFilesMap(aSourceFiles);
}
