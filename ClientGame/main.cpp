#include "pch.h"
#include <CogClientGame.h>

int main()
{
	gThreadID = 1;

	CogClientGame game;
	game.Run();

	return 0;
}
