#include "ClientApplicationPch.h"
#include "Camera.h"
#include "RenderTarget.h"
#include "ClientGame.h"
#include "RenderEngine.h"

void Camera::Created()
{
	Base::Created();

	RenderTarget& renderTarget = *SetRenderTarget(NewChild<RenderTarget>());
	
	ClientGame& clientGame = GetProgram<ClientGame>();
	RenderEngine& renderer = *clientGame.GetRenderer();
	renderer.OnBackbufferRecreated.Subscribe(renderTarget, &RenderTarget::SetRenderTexture);
	renderTarget.SetRenderTexture(renderer.GetBackbuffer());
}

void Camera::Destroyed()
{
}
