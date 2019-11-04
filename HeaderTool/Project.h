#pragma once

enum class ProjectType : uint8
{
	Library,
	Executable
};

struct Project
{
	Project(StringView aSolutionDirectory, StringView aFilePath);

	void ResolveReferences(const Map<String, Project*>& aProjects);

	String projectName;

	Array<String> namesOfReferences;
	Array<Project*> references;

	ProjectType projectType;
	bool preprocess = false;
};
