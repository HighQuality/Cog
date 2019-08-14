#pragma once

#include "../Core/pch.h"

using ChunkIndex = u8;

#define WITH_CLIENT 1

bool IsInGameThread();

#define CHECK_COMPONENT_INCLUDED(Type) do { static_assert(IsComplete<Type>, "Component header must be included"); } while (false)

#include <Cog/Casts.h>
#include <Cog/Entity.h>
#include <Cog/EntityInitializer.h>
#include <Cog/Component.h>
#include <Cog/Pointer.h>
#include <Cog/FrameData.h>

#if WITH_CLIENT

#include <d3d11_1.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

void SetResourceDebugName(ID3D11DeviceChild* resource, StringView aDebugName);

#define CheckDXError(e) do { HRESULT _res = e; if (FAILED(_res)) { FATAL(L"D3D11 Expression Failed:\n%", L"" #e); } } while (false)

#endif // WITH_CLIENT
