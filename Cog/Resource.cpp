#include "pch.h"
#include "Resource.h"
#include "ResourceManager.h"

Array<u8> Resource::ReadEntireFile(const StringView& aFile)
{
	FILE* f;

	if (_wfopen_s(&f, aFile.GetData(), L"rb"))
		FATAL(L"_wfopen_s failed");

	fseek(f, 0, SEEK_END);
	const long size = ftell(f);
	fseek(f, 0, SEEK_SET);

	Array<u8> data;
	data.Resize(CastBoundsChecked<long>(size));
	fread(data.GetData(), size, 1, f);

	if (fclose(f))
		FATAL(L"fclose failed");

	return data;
}

void Resource::RegisterCallback(const Function<void(Resource&)>& aFunctionCallback)
{
}
