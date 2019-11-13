#pragma once

class Directory;

struct DocumentTemplates
{
	void ReadFromDirectory(Directory& aDirectory);

	void CheckIsLoaded() const;

	String solutionTemplate;
	String libraryProjectTemplate;
	String executableProjectTemplate;

	String developmentProjectTemplate;
	String nmakeDebugUserFileTemplate;
	String generatedHeaderTemplate;
	String generatedSourceTemplate;

	bool isLoaded = false;
};

