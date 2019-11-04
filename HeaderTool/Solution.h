#pragma once
#include <Memory/UniquePtr.h>
#include "Project.h"

struct Solution
{
	Solution(StringView aSolutionDirectory);

	Array<UniquePtr<Project>> projects;
	String projectName;
};
