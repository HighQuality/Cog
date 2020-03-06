#pragma once
#include "ResourceManager.generated.h"

COGTYPE()
class ResourceManager : public Singleton
{
	GENERATED_BODY;

public:
	bool Starting() override;
	void ShuttingDown() override;
};
