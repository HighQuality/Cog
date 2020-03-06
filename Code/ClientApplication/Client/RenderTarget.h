#pragma once
#include "RenderTexture.h"
#include "RenderTarget.generated.h"

COGTYPE()
class RenderTarget : public Object
{
	GENERATED_BODY;

public:
	void SetRenderTexture(const std::shared_ptr<RenderTexture>& aNewRenderTexture);

private:
	COGPROPERTY(Ptr<RenderTexture> RenderTexture);
	COGPROPERTY(Vec2 Size, PublicRead);
	COGPROPERTY(i32 Width = 0, PublicRead);
	COGPROPERTY(i32 Height = 0, PublicRead);
};
