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

void CogServerGame::Tick(const Time& aDeltaTime)
{
	Base::Tick(aDeltaTime);
}

void CogServerGame::NewWidgetCreated(Widget& aWidget)
{
	FATAL(L"The server can not create widgets.");
}
