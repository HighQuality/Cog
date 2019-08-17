#include "pch.h"
#include "CogServerGame.h"

CogServerGame::CogServerGame()
{
}

CogServerGame::~CogServerGame()
{
}

bool CogServerGame::ShouldKeepRunning() const
{
	return true;
}

void CogServerGame::SynchronizedTick(const Time& aDeltaTime)
{
	Base::SynchronizedTick(aDeltaTime);
}

void CogServerGame::NewWidgetCreated(Widget& aWidget)
{
	FATAL(L"The server can not create widgets.");
}
