#include "pch.h"
#include "Program.h"
#include <Client/ClientGame.h>

int main()
{
	Program::Create();

	ClientGame game;
	game.Run();

	Program::Destroy();
}
