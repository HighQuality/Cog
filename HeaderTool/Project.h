#pragma once

class Directory;

enum class ProjectType : uint8
{
	Library,
	Executable
};

struct Project
{
	Project(Directory* aProjectDirectory);

	void ResolveReferences(const Map<String, Project*>& aProjects);
	void GenerateProjectFile(StringView aProjectTemplate) const;

	void GatherLibraryPaths(Map<StringView, u8>& aLibraryPaths) const;
	void GatherLinkDependencies(Map<StringView, u8>& aLinkDependencies) const;
	void GatherIncludePaths(Map<StringView, u8>& aIncludePaths) const;
	void GatherProjectReferences(Map<Project*, u8>& aProjectReferences) const;

	String projectName;
	String projectGuid;

	Array<String> namesOfReferences;
	Array<Project*> references;

	Array<String> extraIncludePaths;

	Array<String> linkDependencies;
	Array<String> extraLibraryPaths;

	Array<String> headerFiles;
	Array<String> sourceFiles;

	Directory* directory;

	ProjectType projectType;
	bool preprocess = false;
};
