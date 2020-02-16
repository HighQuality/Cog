#pragma once
#include "RenderTexture.h"
#include "DrawCall.h"
#include "RenderTarget.generated.h"

COGTYPE()
class RenderTarget : public Object
{
	GENERATED_BODY;

public:
	void SetRenderTexture(const std::shared_ptr<RenderTexture>& aNewRenderTexture);

	void Submit(DrawCall aDrawCall)
	{
		DrawCallList().Submit(Move(aDrawCall));
	}

private:
	COGPROPERTY(EventList<DrawCall> DrawCallList, DirectAccess);

	COGPROPERTY(Ptr<RenderTexture> RenderTexture);
	COGPROPERTY(Vec2 Size, PublicRead);
	COGPROPERTY(i32 Width = 0, PublicRead);
	COGPROPERTY(i32 Height = 0, PublicRead);
};
