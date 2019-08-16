#pragma once

class TestComponent : public InheritComponent<Component>
{
public:
	void Tick(const FrameData& aTickData) override;
};

