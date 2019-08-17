#pragma once
#include <Cog/TestComponent.h>

class ClientTestComponent : public InheritComponent<TestComponent>
{
public:

protected:
	void Initialize() override;

};

