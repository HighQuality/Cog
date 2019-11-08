#pragma once
#include <Memory/UniquePtr.h>
#include <Filesystem/Directory.h>
#include "Project.h"

struct Solution
{
	Solution(StringView aSolutionDirectory);

	void GenerateSolutionAndProjects() const;

	void GenerateSolutionFile(StringView aSolutionTemplate) const;
	
	Array<UniquePtr<Project>> projects;
	String solutionName;
	String solutionGuid;
	String vsCppProjectTypeGuid;
	UniquePtr<Directory> directory;

	String solutionFile;

	String solutionTemplate;
	String libraryProjectTemplate;
	String executableProjectTemplate;
};
