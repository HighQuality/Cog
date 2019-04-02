#include "pch.h"
#include "ClientGame.h"
#include <Program.h>

int main()
{
	Program::Create();

	ClientGame game;
	game.Run();

	Program::Destroy();

	return 0;
}
