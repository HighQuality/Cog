#include "pch.h"
#include "ClientGame.h"
#include <Fiber.h>
#include <Stopwatch.h>

int main()
{
	{
		Fiber f;
		f.StartWork([](void*)
		{
			for (i32 i = 0; i < 3; ++i)
			{
				Println(L"Hello from fiber ", i);
				Fiber::YieldExecution();
			}
		}, nullptr);

		i32 i = 0;
		while (f.Continue())
		{
			Println(L"Hello from main ", i);
			++i;
		}
	}

	std::cin.get();

	ClientGame game;
	game.Run();

	return 0;
}
