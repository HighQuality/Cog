#include "ClientApplicationPch.h"
#include "PixelShader.h"
#include "RenderEngine.h"

PixelShader::PixelShader(RenderEngine& aRenderer, const StringView& aFileName)
	: Shader(aRenderer)
{
	ComPtr<ID3D10Blob> blob = LoadBlobFromShaderFile(aFileName, "ps_main", "ps_5_0", true);

	if (blob)
	{
		aRenderer.GetDevice()->CreatePixelShader(blob->GetBufferPointer(), blob->GetBufferSize(), nullptr, myShader.ReleaseAndGetAddressOf());
	}
}

PixelShader::~PixelShader()
{
}

void PixelShader::Bind()
{
	if (!ENSURE(myShader))
		return;

	myRenderer->GetContext()->PSSetShader(myShader.Get(), nullptr, 0);
}
