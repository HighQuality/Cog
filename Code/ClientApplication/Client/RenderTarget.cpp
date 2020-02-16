#include "ClientApplicationPch.h"
#include "RenderTarget.h"

void RenderTarget::SetRenderTexture(const std::shared_ptr<RenderTexture>& aNewRenderTexture)
{
	SetWidth(aNewRenderTexture->GetWidth());
	SetHeight(aNewRenderTexture->GetHeight());

	SetSize(Vec2(static_cast<f32>(GetWidth()), static_cast<f32>(GetHeight())));
}
