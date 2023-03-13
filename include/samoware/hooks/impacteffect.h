
#pragma once

#include "cfw/basefunchook.h"

#include "../sdk/effects.h"

namespace hooks::impact {
void __cdecl ImpactFunctionHookFunc(const CEffectData& data);
void hook();
void unHook();
}

