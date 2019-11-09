#pragma once
#include "Defer.h"

#define bring_into_scope(localVariable, memberVariable) auto localVariable = memberVariable; defer { memberVariable = localVariable; }
