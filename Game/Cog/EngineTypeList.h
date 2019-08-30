#pragma once
#include "TypeList.h"

class EngineTypeList : public TypeList
{
public:
	using Base = TypeList;

protected:
	void RegisterTypes() override;
};

