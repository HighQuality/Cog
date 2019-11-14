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
		myDrawCallList.Submit(Move(aDrawCall));
	}

	FORCEINLINE i32 GetWidth() const { return myWidth; }
	FORCEINLINE i32 GetHeight() const { return myHeight; }
	FORCEINLINE Vec2 GetSize() const { return mySize; }

private:
	EventList<DrawCall> myDrawCallList;

	std::shared_ptr<RenderTexture> myRenderTexture;
	Vec2 mySize;
	i32 myWidth = 0;
	i32 myHeight = 0;
};
