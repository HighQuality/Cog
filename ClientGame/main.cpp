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

	for (i32 i = 0; i < 100; ++i)
	{
		for (i32 j = 0; j < files.GetLength(); ++j)
		{
			program.QueueWork([](void* aArg)
			{
				StringView path = *static_cast<StringView*>(aArg);
				Await<ReadFileAwaitable>(path);
			}, &files[j]);
		}
	}

	program.Run();

	// ClientGame game;
	// game.Run();

	system("pause");

	return 0;
}
