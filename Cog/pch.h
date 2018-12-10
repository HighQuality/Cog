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

// 0 = unassigned, 1 = game thread
inline u16 gThreadID = 0;

FORCEINLINE bool IsInGameThread()
{
	return gThreadID == 1;
}
