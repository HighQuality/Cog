#include "pch.h"
#include <ClientCogGame.h>

int main()
{
	gThreadID = 1;

	ClientCogGame game;
	game.Run();

	return 0;
}
