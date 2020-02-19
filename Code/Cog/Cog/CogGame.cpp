#include "CogPch.h"
#include "CogGame.h"
#include "ResourceManager.h"

bool CogGame::Starting()
{
	if (!Base::Starting())
		return false;

	SetFrameData(MakeUnique<FrameData>());
	return true;
}

void CogGame::ShuttingDown()
{
	Base::ShuttingDown();
}

void CogGame::SynchronizedTick(const Time& aDeltaTime)
{
	GetSingleton<ResourceManager>().Tick();

	UpdateFrameData(*GetFrameData(), aDeltaTime);
}

void CogGame::UpdateFrameData(FrameData& aData, const Time& aDeltaTime)
{
	aData.deltaTime = aDeltaTime;
}
