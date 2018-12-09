#pragma once

class InputLayout
{
public:
	InputLayout();
	~InputLayout();

	void Add(const char* aSemanticName, i32 aSemanticIndex, DXGI_FORMAT aFormat, i32 aInputSlot = 0);
	void AddPerInstance(const char* aSemanticName, i32 aSemanticIndex, DXGI_FORMAT aFormat, i32 aInputSlot = 0);

	const D3D11_INPUT_ELEMENT_DESC* GetBeginLayout() const;
	i32 GetLayoutCount() const;

private:
	Array<D3D11_INPUT_ELEMENT_DESC> myDescription;
	i32 myPerVertexCount;
	i32 myPerInstanceCount;
};