#pragma once

class TestComponent : public Component
{
public:
	void Tick(Time aDeltaTime) override;

private:
	void Callback();
};

