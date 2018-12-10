#pragma once

#include "../Game/pch.h"

#include <d3d11_1.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

void SetResourceDebugName(ID3D11DeviceChild* resource, StringView aDebugName);
