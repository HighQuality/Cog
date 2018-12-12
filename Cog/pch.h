#pragma once

#pragma warning ( disable : 4065 )

#include "../Utilities/pch.h"

// These are wrapped in ifndef so they can be overriden via compiler settings

#ifndef PRODUCTION
#define PRODUCTION 0
#endif

#ifndef ARRAY_BOUNDSCHECK
#define ARRAY_BOUNDSCHECK 1
#endif

bool IsInGameThread();

#include "Casts.h"
#include "Object.h"
#include "ObjectInitializer.h"
#include "Component.h"
#include "Pointer.h"
