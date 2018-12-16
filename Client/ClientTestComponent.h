#pragma once
#include <TestComponent.h>

class ClientTestComponent : public InheritComponent<TestComponent>
{
public:

protected:
	void Initialize() override;
	void Callback() override;

};

