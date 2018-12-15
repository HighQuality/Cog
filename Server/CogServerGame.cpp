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

BaseWidgetFactory& CogServerGame::FindOrCreateWidgetFactory(const TypeID<Widget>& aWidgetType, const FunctionView<BaseWidgetFactory*()>& aFactoryCreator)
{
	FATAL(L"The server may not create widgets");
}
