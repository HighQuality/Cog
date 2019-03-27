#pragma once
#include <Component.h>

class Transform2D;

class SpriteComponent : public InheritComponent<Component>
{
public:
	void Tick(const FrameData& aTickData) override;

	void GpuExec();

protected:
	void ResolveDependencies(EntityInitializer& aInitializer) override;

private:
	Transform2D* myTransform = nullptr;
};
