#include "pch.h"
#include <iostream>
#include <Utilities/Program.h>
#include <ClientGame/ClientGame.h>

int main()
{
	Program::Create();

	ClientGame game;
	game.Run();

	Program::Destroy();
}
