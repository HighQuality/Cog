#include "pch.h"
#include "ClientTestComponent.h"
#include "ImageWidget.h"

void ClientTestComponent::Initialize()
{
	Base::Initialize();

	Println(L"Initializing ClientTestComponent");

	CreateWidget<ImageWidget>();
}

void ClientTestComponent::Callback()
{
	// Println(L"Client callback");
}
