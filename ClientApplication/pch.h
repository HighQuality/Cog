#pragma once

#include "../Game/exported_pch.h"

#include <d3d11_1.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

void SetResourceDebugName(ID3D11DeviceChild* resource, StringView aDebugName);

#define CheckDXError(e) do { HRESULT _res = e; if (FAILED(_res)) { FATAL(L"D3D11 Expression Failed:\n%", L"" #e); } } while (false)
