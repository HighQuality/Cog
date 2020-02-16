#pragma once
#include "RenderTarget.h"
#include "Camera.generated.h"

COGTYPE()
class Camera : public Object
{
	GENERATED_BODY;

public:
	void Created() override;
	void Destroyed() override;

private:
	COGPROPERTY(Ptr<RenderTarget> RenderTarget);
};