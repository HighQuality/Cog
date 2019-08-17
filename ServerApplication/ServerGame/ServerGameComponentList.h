#pragma once
#include <Server/ServerComponentList.h>

class ServerGameComponentList : public ServerComponentList
{
public:
	using Base = ServerComponentList;

protected:
	void RegisterComponents() override;
};

