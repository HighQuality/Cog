#pragma once

class TestComponent : public InheritComponent<Component>
{
public:
	void Tick(Time aDeltaTime) override;

private:
	virtual void Callback();
};

