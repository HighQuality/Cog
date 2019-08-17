#pragma once
#include <Client/ClientComponentList.h>

class ClientGameComponentList final : public ClientComponentList
{
public:
	using Base = ClientComponentList;

protected:
	void RegisterComponents() override;
};

