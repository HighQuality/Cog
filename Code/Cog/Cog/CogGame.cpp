#include "CogPch.h"
#include "CogGame.h"

bool CogGame::Starting()
{
	if (!Base::Starting())
		return false;

	SetFrameData(MakeUnique<FrameData>());

	SetResourceManager(NewObject<ResourceManager>(nullptr));
	return true;
}

void CogGame::ShuttingDown()
{
	GetResourceManager()->Destroy();

	Base::ShuttingDown();
}

void CogGame::SynchronizedTick(const Time& aDeltaTime)
{
	if (GetResourceManager())
		GetResourceManager()->Tick();

	UpdateFrameData(*GetFrameData(), aDeltaTime);
}

void CogGame::UpdateFrameData(FrameData& aData, const Time& aDeltaTime)
{
	aData.deltaTime = aDeltaTime;
}
