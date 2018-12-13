#pragma once
#include <ClientComponentList.h>

class ClientGameComponentList final : public ClientComponentList
{
public:
	using Base = ClientComponentList;

protected:
	void RegisterComponents() override;
};

