#pragma once
#include <Cog/Component.h>
// #include "SpriteComponent.gen.h"

class Transform2D;

COGTYPE()
class SpriteComponent : public InheritComponent<Component>
{
	GENERATED_BODY();

public:
	void Tick(const FrameData& aTickData) override;

	void GpuExec();

protected:
	void ResolveDependencies(EntityInitializer& aInitializer) override;

private:
	Transform2D* myTransform = nullptr;
};
