#include "pch.h"
#include "ClientGame.h"
#include <Program.h>
#include <Fiber.h>
#include <ReadFileAwaitable.h>

std::atomic<i32> sizeSum = 0;
std::atomic<i32> awaitablesExecuted = 0;
std::atomic<i32> awaitablesDone = 0;

i32 FileSize(StringView aPath)
{
	if (true)
	{
		std::ifstream f(aPath.ToStdWString());
		f.seekg(0, std::ios::end);
		return CastBoundsChecked<i32>(f.tellg());
	}
	else
	{
		std::ifstream fileStream(aPath.ToStdWString());

		if (!fileStream.good())
			FATAL(L"Failed to open file \"%\"", aPath);

		fileStream.seekg(0, std::ios::end);
		Array<u8> data;
		data.Resize(static_cast<i32>(fileStream.tellg()));
		fileStream.seekg(0, std::ios::beg);
		fileStream.read(reinterpret_cast<char*>(data.GetData()), data.GetLength());
		return data.GetLength();
	}
}

int main()
{
	Program& program = Program::Create();

	Array<StringView> files(
		{
			L"ClientGame.cpp",
			L"ClientGame.h",
			L"pch.h",
			L"ClientGameComponentList.cpp"
		});

	i32 expectedSize = 0;

	for (i32 i = 0; i < 10; ++i)
	{
		for (StringView& path : files)
		{
			expectedSize += FileSize(path);

			program.QueueWork([](void* aArg)
			{
				StringView filePath = *static_cast<StringView*>(aArg);

				// Println(L"Loading file %...", filePath);

				ReadFileAwaitable readFileTask(filePath);
				Await awaitable(readFileTask);

				awaitablesExecuted.fetch_add(1);

				awaitable.Execute();

				Array<u8> data = readFileTask.RetrieveReturnedData();

				// Println(L"Read % bytes from ", data.GetLength(), filePath);

				sizeSum.fetch_add(data.GetLength());

				awaitablesDone.fetch_add(1);

			}, &path);
		}
	}

	program.Run();

	i32 executed = awaitablesExecuted.load();
	i32 done = awaitablesDone.load();

	Println(L"% (Received)", sizeSum);
	Println(L"% (Expected)", expectedSize);
	Println(L"% / %", done, executed);

	// ClientGame game;
	// game.Run();

	system("pause");

	return 0;
}
