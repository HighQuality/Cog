#include "pch.h"
#include "Component.h"
#include "CogScene.h"

CogGame& Component::GetCogGame() const
{
	Object& object = GetObject();
	CogScene& scene = object.GetCogScene();
	return scene.GetCogGame();
}
