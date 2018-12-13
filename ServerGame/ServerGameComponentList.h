#pragma once
#include <ServerComponentList.h>

class ServerGameComponentList : public ServerComponentList
{
public:
	using Base = ServerComponentList;

protected:
	void RegisterComponents() override;
};

