#include "pch.h"
#include "SpriteComponent.h"
#include <Cog/Transform2D.h>
#include "GpuCommand.h"

void SpriteComponent::ResolveDependencies(EntityInitializer& aInitializer)
{
	Base::ResolveDependencies(aInitializer);

	myTransform = &aInitializer.FindOrAddComponent<Transform2D>();
}

void SpriteComponent::Tick(const FrameData& aTickData)
{
	GpuCommand command { };
	command.type = GpuCommandType::DrawSprite;
	command.drawSpriteData.sprite = this;

	aTickData.gpuCommands->Submit(command);
}

void SpriteComponent::GpuExec()
{
	Println(L"GpuExec SpriteComponent");
}
