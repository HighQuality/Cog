#include "CogPch.h"
#include "Singleton.h"

bool Singleton::Starting()
{
	SetIsBaseCalled(true);
	return true;
}

void Singleton::ShuttingDown()
{
	SetIsBaseCalled(true);
}
