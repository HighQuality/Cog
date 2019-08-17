#include "pch.h"
#include "RenderTarget.h"

void RenderTarget::SetRenderTexture(const std::shared_ptr<RenderTexture>& aNewRenderTexture)
{
	myWidth = aNewRenderTexture->GetWidth();
	myHeight = aNewRenderTexture->GetHeight();

	mySize = Vec2(static_cast<f32>(myWidth), static_cast<f32>(myHeight));
}
