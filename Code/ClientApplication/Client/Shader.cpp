#include "ClientApplicationPch.h"
#include "Shader.h"
#include <D3DCompiler.h>

Shader::Shader(RenderEngine& aRenderer)
{
	myRenderer = &aRenderer;
}

Shader::~Shader()
{
}

ComPtr<ID3D10Blob> Shader::LoadBlobFromShaderFile(const StringView& aFileName, const char* aEntryPoint, const char* aCompileProfile, const bool aAbortOnError, const bool aForceOptimization /*= false*/)
{
	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
	flags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;

	if (!aForceOptimization && !PRODUCTION)
	{
		flags |= D3DCOMPILE_DEBUG;
		flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	}
	else
	{
		flags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
		flags |= D3DCOMPILE_AVOID_FLOW_CONTROL;
	}

	LPCSTR profile = aCompileProfile;
	const D3D_SHADER_MACRO defines[] =
	{
		NULL, NULL
	};

	ComPtr<ID3DBlob> errorBlob = nullptr;
	ComPtr<ID3DBlob> shaderBlob = nullptr;
	
	HRESULT hr = D3DCompileFromFile(aFileName.ToStdWString().c_str(), defines, nullptr,
		aEntryPoint, profile,
		flags, 0, &shaderBlob, &errorBlob);

	if (FAILED(hr))
	{
		String errorMessage;

		if (errorBlob)
		{
			errorMessage = String(static_cast<const char*>(errorBlob->GetBufferPointer()));
			errorBlob->Release();
			errorBlob = nullptr;
		}
		else if (hr == HRESULT_FROM_WIN32(ERROR_PATH_NOT_FOUND))
		{
			errorMessage = Format(L"Could not load shader %, could not find file:\n%", aEntryPoint, aFileName);
		}
		
		if (aAbortOnError)
		{
			FATAL(L"%", errorMessage);
		}
		else
		{
			Println(errorMessage);
		}

		return nullptr;
	}

	return shaderBlob;
}
