#include "pch.h"
#include "Program.h"
#include <ClientGame/ClientGame.h>

int main()
{
	Program::Create();

	ClientGame game;
	game.Run();

	Program::Destroy();
}
