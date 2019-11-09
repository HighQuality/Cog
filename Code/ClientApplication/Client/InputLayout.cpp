#include "ClientApplicationPch.h"
#include "InputLayout.h"

InputLayout::InputLayout()
{
	myPerVertexCount = 0;
	myPerInstanceCount = 0;
}

InputLayout::~InputLayout()
{
}

void InputLayout::Add(const char* aSemanticName, const i32 aSemanticIndex, const DXGI_FORMAT aFormat, const i32 aInputSlot)
{
	if (myPerInstanceCount > 0)
		FATAL(L"Per Instance layout descriptions must be added last!");

	D3D11_INPUT_ELEMENT_DESC desc { };

	desc.SemanticName = aSemanticName;
	desc.SemanticIndex = aSemanticIndex;
	desc.Format = aFormat;
	desc.InputSlot = aInputSlot;
	desc.AlignedByteOffset = myDescription.GetLength() > 0 ? D3D11_APPEND_ALIGNED_ELEMENT : 0;
	desc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	desc.InstanceDataStepRate = 0;

	myPerVertexCount++;
	myDescription.Add(desc);
}

void InputLayout::AddPerInstance(const char* aSemanticName, const i32 aSemanticIndex, const DXGI_FORMAT aFormat, i32 aInputSlot)
{
	D3D11_INPUT_ELEMENT_DESC desc { };

	desc.SemanticName = aSemanticName;
	desc.SemanticIndex = aSemanticIndex;
	desc.Format = aFormat;
	desc.InputSlot = aInputSlot;
	desc.AlignedByteOffset = myPerInstanceCount > 0 ? D3D11_APPEND_ALIGNED_ELEMENT : 0;
	desc.InputSlotClass = D3D11_INPUT_PER_INSTANCE_DATA;
	desc.InstanceDataStepRate = 1;

	myPerInstanceCount++;
	myDescription.Add(desc);
}

const D3D11_INPUT_ELEMENT_DESC* InputLayout::GetBeginLayout() const
{
	if (myDescription.GetLength() == 0)
		return nullptr;

	return myDescription.GetData();
}

i32 InputLayout::GetLayoutCount() const
{
	return myDescription.GetLength();
}
