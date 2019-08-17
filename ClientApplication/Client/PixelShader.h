#pragma once
#include "Shader.h"

class RenderEngine;

class PixelShader : public Shader
{
public:
	explicit PixelShader(RenderEngine& aRenderer, const StringView& aFileName);
	~PixelShader();

	void Bind();

private:
	ComPtr<ID3D11PixelShader> myShader;
};

