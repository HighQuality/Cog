#include "pch.h"
#include "ClientGame.h"
#include <Program.h>
#include <Fiber.h>
#include <AwaitTime.h>
#include <ReadFileAwaitable.h>

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

	for (i32 i = 0; i < 1; ++i)
	{
		for (StringView& path : files)
		{
			program.QueueWork([](void* aArg)
			{
				StringView path = *static_cast<StringView*>(aArg);

				ReadFileAwaitable FileReadTask(path);
				Array<u8> data = Await(FileReadTask);

				Println(L"Returned from reading % bytes from ", data.GetLength(), path);
				
				Stopwatch w;
				Await(AwaitTime(Time::Seconds(1.f)));

				Println(L"Waited % seconds; data for % is % bytes", w.GetElapsedTime().Seconds(), path, data.GetLength());

			}, &path);
		}
	}

	program.Run();

	// ClientGame game;
	// game.Run();

	system("pause");

	return 0;
}
