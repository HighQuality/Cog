#include "pch.h"
#include "ClientGame.h"
#include <Program.h>
#include <Fiber.h>
#include <AwaitTime.h>

int main()
{
	Program::Create();

	for (i32 i = 0; i < 1000; ++i)
	{
		Program::Get().QueueWork([](void*)
			{
				Stopwatch w;
				Await<AwaitTime>(Time::Seconds(2.f));
				// Println(L"Yield lasted % seconds", w.GetElapsedTime().Seconds());
			}, nullptr);
	}

	Program::Get().Run();

	// ClientGame game;
	// game.Run();

	system("pause");

	return 0;
}
