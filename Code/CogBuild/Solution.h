#pragma once
#include <Memory/UniquePtr.h>
#include <Filesystem/Directory.h>
#include "Project.h"
#include "DocumentTemplates.h"

struct SolutionDocumentProjectReference
{
	SolutionDocumentProjectReference(StringView aGuid, StringView aProjectName, StringView aProjectFilePath)
		: guid(aGuid), projectName(aProjectName), projectFilePath(aProjectFilePath)
	{ }

	SolutionDocumentProjectReference() = default;

	StringView guid;
	StringView projectName;
	StringView projectFilePath;
};

struct Solution
{
	Solution(StringView aSolutionDirectory);

	void ReadTemplates(StringView aTemplateDirectory);

	void GenerateBuildProjects() const;

	void GenerateSolutionFile(StringView aSolutionFilePath, ArrayView<SolutionDocumentProjectReference> aProjects) const;

	void GenerateDevelopmentProjects(StringView aBuildToolPath) const;
	void GenerateDevelopmentMainProjectFile(StringView aBuildToolPath) const;
	
	bool GenerateCode();
	
	Array<UniquePtr<Project>> projects;
	String solutionName;
	// Shared between build and development solution
	String solutionGuid;
	String developmentMainProjectName;
	String developmentMainProjectFile;
	String developmentMainProjectGuid;
	String vsCppProjectTypeGuid;
	UniquePtr<Directory> directory;
	Directory* codeDirectory;

	String buildSolutionFile;
	String developmentSolutionFile;

	DocumentTemplates templates;
};
