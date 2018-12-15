#pragma once

class TestComponent : public InheritComponent<Component>
{
public:
	void Tick(Time aDeltaTime) override;

protected:
	virtual void Callback();
};

