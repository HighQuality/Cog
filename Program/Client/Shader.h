#pragma once

class RenderEngine;

class Shader
{
public:
	virtual ~Shader();

protected:
	Shader(RenderEngine& aRenderer);

	RenderEngine* myRenderer;

	ComPtr<ID3D10Blob> LoadBlobFromShaderFile(const StringView& aFileName, const char* aEntryPoint, const char* aCompileProfile, bool aAbortOnError, bool aForceOptimization = false);
};

