#include "pch.h"
#include "ClientTestComponent.h"
#include "ImageWidget.h"

void ClientTestComponent::Initialize()
{
	Base::Initialize();

	CreateWidget<ImageWidget>();
}

void ClientTestComponent::Callback()
{
	// Println(L"Client callback");
}
