#include "pch.h"
#include "ClientGame.h"
#include <Program.h>
#include <Fiber.h>
#include <AwaitTime.h>
#include <ReadFileAwaitable.h>

int main()
{
	Program& program = Program::Create();

	// for (i32 i = 0; i < 1000; ++i)
	{
		// program.QueueWork([](void*)
		// {
		// 	Stopwatch w;
		// 	Await<AwaitTime>(Time::Seconds(RandFloat(0.5f, 5.f)));
		// 	Println(L"Yield lasted % seconds", w.GetElapsedTime().Seconds());
		// }, nullptr);

		program.QueueWork([](void*)
		{
			Await<ReadFileAwaitable>(L"main.cpp");

			Sleep(10);

			Println(L"Returned to work");

			Sleep(10);
		}, nullptr);
	}

	program.Run();

	// ClientGame game;
	// game.Run();

	system("pause");

	return 0;
}
