#include "pch.h"
#include "ClientGame.h"

int main()
{
	gThreadID = 1;

	ClientGame game;
	game.Run();

	return 0;
}
