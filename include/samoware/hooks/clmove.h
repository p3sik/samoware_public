
#pragma once

#include "cfw/basefunchook.h"

class CStudioHdr;

namespace hooks {
void __fastcall CL_MoveHookFunc(float accumulated_extra_samples, bool bFinalTick);

class CL_MoveHook : public cfw::BaseFunctionHook<CL_MoveHookFunc, cfw::HookType::STANDARD, 0>, public cfw::Singleton<CL_MoveHook> {
private:
	const char* GetHookName() const { return "CL_Move"; }

	void* GetHookTarget() {
		return (void*)cfw::findPattern("engine.dll", "40 55 53 48 8D AC 24 38 F0 FF FF B8 C8 10 00 00 E8 ?? ?? ?? ?? 48 2B E0 0F 29 B4 24 B0 10 00 00");
	}

public:
	CL_MoveHook(token) {}
};
}

