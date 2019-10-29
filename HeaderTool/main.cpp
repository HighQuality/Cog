#include "pch.h"
#include <External/json.h>
#include <fstream>

int wmain(int argc, wchar_t* argv[], wchar_t* envp[])
{
	if (argc < 2)
	{
		Println(L"Too few inputs!");
		return 1;
	}

	std::ifstream f(argv[1]);
	if (!f.is_open())
	{
		Println(L"Failed to open configuration file %", StringView(argv[1]));
		return 2;
	}

	nlohmann::json document;
	f >> document;

	Println(L"", document["type-list"].get<std::string>().c_str());

	return 0;
}
