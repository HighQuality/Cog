#include "pch.h"
#include "ClientTestComponent.h"
#include "ImageWidget.h"

void ClientTestComponent::Initialize()
{
	Base::Initialize();

	CreateWidget<ImageWidget>();
}
