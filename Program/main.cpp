#include "pch.h"
#include <iostream>
#include <Utilities/Program.h>
#include <ClientGame/ClientGame.h>

void TestCore();

int main()
{
	TestCore();
	std::cin.get();

	Program::Create();

	ClientGame game;
	game.Run();

	Program::Destroy();
}
