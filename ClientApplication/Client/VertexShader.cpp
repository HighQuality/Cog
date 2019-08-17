#include "pch.h"
#include "VertexShader.h"
#include "RenderEngine.h"
#include "InputLayout.h"

VertexShader::VertexShader(RenderEngine& aRenderer, const StringView& aFileName, const InputLayout& aInputLayout)
	: Shader(aRenderer)
{
	ComPtr<ID3D10Blob> blob = LoadBlobFromShaderFile(aFileName, "vs_main", "vs_5_0", true);

	if (blob)
	{
		CheckDXError(
			aRenderer.GetDevice()->CreateVertexShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, myShader.ReleaseAndGetAddressOf())
		);

		CheckDXError(
			aRenderer.GetDevice()->CreateInputLayout(aInputLayout.GetBeginLayout(), aInputLayout.GetLayoutCount(), blob->GetBufferPointer(), blob->GetBufferSize(), myInputLayout.ReleaseAndGetAddressOf())
		);
	}
}

VertexShader::~VertexShader()
{
}

void VertexShader::Bind()
{
	if (!ENSURE(myShader && myInputLayout))
		return;

	myRenderer->GetContext()->VSSetShader(myShader.Get(), nullptr, 0);
	myRenderer->GetContext()->IASetInputLayout(myInputLayout.Get());
}
