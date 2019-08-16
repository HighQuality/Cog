#include "pch.h"
#include "Program.h"
#include <ServerGame/ServerGame.h>

int main()
{
	Program::Create();

	ServerGame game;
	game.Run();

	Program::Destroy();
}
