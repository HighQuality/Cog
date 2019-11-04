#include "pch.h"
#include "Solution.h"
#include <External\json.h>

Solution::Solution(const StringView aSolutionDirectory)
{
	Println(L"Parsing solution...");
	
	const String filePath = Format(L"%/HeaderTool.json", aSolutionDirectory);

	// TODO: Change to wide string
	std::ifstream f(filePath.View().ToStdString());

	if (!f.is_open())
		FATAL(L"Failed to open solution file %", filePath);

	json document;
	f >> document;

	projectName = String(document["projectName"].get<std::string>().c_str());

	Map<String, Project*> projectMap;

	for (const auto& projectDirectory : document["projects"].get<json::array_t>())
	{
		Project* project = projects.Add(MakeUnique<Project>(aSolutionDirectory, String(projectDirectory.get<std::string>().c_str()).View()));

		Project*& projectPtr = projectMap.FindOrAdd(project->projectName.View());

		if (projectPtr)
			FATAL(L"Project % is already defined.", project->projectName);

		projectPtr = project;
	}

	for (auto& project : projects)
		project->ResolveReferences(projectMap);
}
