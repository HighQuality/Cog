#pragma once
#include "HeaderParser.h"

class Directory;
class File;
struct DocumentTemplates;

enum class ProjectType : uint8
{
	Library,
	Executable
};

struct Project
{
	Project(Directory* aProjectDirectory);

	void ResolveReferences(const Map<String, Project*>& aProjects);
	void GenerateBuildProjectFile(StringView aProjectTemplate) const;
	void GenerateDebugDevelopmentProjectFile(StringView aMainProjectFilePath, StringView aMainProjectGuid, const DocumentTemplates& aTemplates) const;
	
	bool ParseHeaders();

	/** These gather functions gather all the properties from this and all referenced projects */
	void GatherLibraryPaths(Map<StringView, u8>& aLibraryPaths) const;
	void GatherLinkDependencies(Map<StringView, u8>& aLinkDependencies) const;
	void GatherIncludePaths(Map<StringView, u8>& aIncludePaths) const;
	void GatherProjectReferences(Map<Project*, u8>& aProjectReferences) const;

	void GatherHeaderFiles(Map<const File*, u8>& aHeaderFiles) const;
	void GatherSourceFiles(Map<const File*, u8>& aSourceFiles) const;

	String projectName;
	String projectGuid;

	String generatedCodeDirectory;

	String buildProjectFile;
	String debugDevelopmentProjectFile;
	String debugDevelopmentUserProjectFile;

	Array<UniquePtr<HeaderParser>> myHeaderParsers;

	Array<String> namesOfReferences;
	Array<Project*> references;

	Array<String> extraIncludePaths;

	Array<String> linkDependencies;
	Array<String> extraLibraryPaths;

	Array<const File*> headerFiles;
	Array<const File*> sourceFiles;

	Directory* directory;

	ProjectType projectType;
	bool preprocess = false;
};
