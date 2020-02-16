#include "CogPch.h"
#include "CogGame.h"

void CogGame::Created()
{
	Base::Created();

	SetFrameData(MakeUnique<FrameData>());

	SetResourceManager(NewChild<ResourceManager>());
}

void CogGame::Destroyed()
{
	GetResourceManager()->Destroy();

	Base::Destroyed();
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
