#include "CogPch.h"
#include "CogGame.h"
#include "ResourceManager.h"

bool CogGame::Starting()
{
	if (!Base::Starting())
		return false;

	return true;
}

void CogGame::ShuttingDown()
{
	Base::ShuttingDown();
}

void CogGame::SynchronizedTick(const f32 aDeltaSeconds)
{
	Base::SynchronizedTick(aDeltaSeconds);
}
