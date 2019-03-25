#include "pch.h"
#include "ClientGame.h"
#include <Program.h>
#include <Fiber.h>
#include <ReadFileAwaitable.h>

std::atomic<i32> sizeSum = 0;

int main()
{
	Program& program = Program::Create();

	Array<StringView> files(
	{
		L"main.cpp",
		L"ClientGame.cpp",
		L"ClientGame.h",
		L"pch.h",
		L"ClientGameComponentList.cpp"
	});

	sizeSum = 0;

	for (i32 i = 0; i < 1; ++i)
	{
		for (StringView& path : files)
		{
			program.QueueWork([](void* aArg)
			{
				StringView filePath = *static_cast<StringView*>(aArg);
				
				//Println(L"Loading file %...", filePath);

				ReadFileAwaitable readFileTask(filePath);
				Await awaitable(readFileTask);
				awaitable.Execute();

				Array<u8> data = readFileTask.RetrieveReturnedData();

				Println(L"Read % bytes from ", data.GetLength(), filePath);

				sizeSum.fetch_add(data.GetLength());

			}, &path);
		}
	}

	program.Run();

	Println(L"% (Received)", sizeSum);
	Println(L"18070 (Expected)");

	// ClientGame game;
	// game.Run();

	system("pause");

	return 0;
}
