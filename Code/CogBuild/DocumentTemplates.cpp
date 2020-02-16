#include "CogBuildPch.h"
#include "DocumentTemplates.h"
#include <Filesystem/Directory.h>
#include <Filesystem/File.h>

static String ReadStringFromFileInDirectory(const StringView aFileName, Directory& aDirectory)
{
	File* file = aDirectory.GetFile(aFileName);
	if (!file)
		FATAL(L"Can't find \"%\" in template directory", aFileName);
	return file->ReadString();
}

void DocumentTemplates::ReadFromDirectory(Directory& aDirectory)
{
	solutionTemplate = ReadStringFromFileInDirectory(L"SolutionTemplate.txt", aDirectory);
	libraryProjectTemplate = ReadStringFromFileInDirectory(L"LibraryProjectTemplate.txt", aDirectory);
	executableProjectTemplate = ReadStringFromFileInDirectory(L"ExecutableProjectTemplate.txt", aDirectory);
	developmentProjectTemplate = ReadStringFromFileInDirectory(L"NmakeTemplate.txt", aDirectory);
	nmakeDebugUserFileTemplate = ReadStringFromFileInDirectory(L"NmakeDebugUserFileTemplate.txt", aDirectory);
	generatedHeaderTemplate = ReadStringFromFileInDirectory(L"GeneratedHeaderTemplate.txt", aDirectory);
	generatedSourceTemplate = ReadStringFromFileInDirectory(L"GeneratedSourceTemplate.txt", aDirectory);

	typeListTemplate = ReadStringFromFileInDirectory(L"TypeListTemplate.txt", aDirectory);
	typeListInvocatorTemplate = ReadStringFromFileInDirectory(L"TypeListInvocatorTemplate.txt", aDirectory);

	executableMainTemplate = ReadStringFromFileInDirectory(L"ExecutableMainTemplate.txt", aDirectory);

	isLoaded = true;
}

void DocumentTemplates::CheckIsLoaded() const
{
	CHECK(isLoaded);
}
