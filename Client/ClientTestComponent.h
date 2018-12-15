#pragma once
#include <TestComponent.h>

class ClientTestComponent : public TestComponent
{
public:

protected:
	void Initialize() override;
	void Callback() override;

};

