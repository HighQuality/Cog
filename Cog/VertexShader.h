#pragma once
#include "Shader.h"

class InputLayout;
class RenderEngine;

class VertexShader : public Shader
{
public:
	VertexShader(RenderEngine& aRenderer, const StringView& aFileName, const InputLayout& aInputLayout);
	~VertexShader();

	void Bind();

private:
	ComPtr<ID3D11VertexShader> myShader;
	ComPtr<ID3D11InputLayout> myInputLayout;
};

