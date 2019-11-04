#include "pch.h"
#include "Project.h"
#include <External\json.h>

Project::Project(const StringView aSolutionDirectory, const StringView aDirectoryName)
{
	projectName = String(aDirectoryName);

	Println(L"Parsing project %...", aDirectoryName);
	
	const String filePath = Format(L"%/%/HeaderTool.json", aSolutionDirectory, aDirectoryName);

	// TODO: Change to wide string
	std::ifstream f(filePath.View().ToStdString());

	if (!f.is_open())
		FATAL(L"Failed to open project file %", filePath);

	json document;
	f >> document;

	if (document.find("references") != document.end())
	{
		const auto& referencesArray = document["references"];
		CHECK(referencesArray.is_array());

		for (const auto& reference : referencesArray.get<json::array_t>())
			namesOfReferences.Add(String(reference.get<std::string>().c_str()));
	}

	preprocess = document["preprocess"].get<bool>();
	preprocess = document["whaa"].get<bool>();

	const std::string typeName = document["type"].get<std::string>();

	if (typeName == "executable")
		projectType = ProjectType::Executable;
	else if (typeName == "library")
		projectType = ProjectType::Library;
	else
		FATAL(L"Unknown project type % in project %", typeName.c_str(), projectName);
}

void Project::ResolveReferences(const Map<String, Project*>& aProjects)
{
	for (const auto& referenceName : namesOfReferences)
	{
		if (Project* referencedProjectName = aProjects.Find(referenceName.View(), nullptr))
			references.Add(referencedProjectName);
		else
			FATAL(L"Can't find referenced project % in project %", referenceName, projectName);
	}
}
