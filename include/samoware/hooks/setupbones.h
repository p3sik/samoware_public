
#pragma once

#include "cfw/basefunchook.h"

#include "../sdk/cbaseentity.h"

class matrix3x4_t;

namespace hooks {
	bool __fastcall SetupBonesHookFunc(CBaseAnimating* self, matrix3x4_t* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime);

	class SetupBonesHook : public cfw::BaseFunctionHook<SetupBonesHookFunc, cfw::HookType::STANDARD, CBaseAnimating::vIndex_SetupBones>, public cfw::Singleton<SetupBonesHook> {
	private:
		const char* GetHookName() const { return "SetupBones"; }

		void* GetHookTarget() {
			// idk where the fuck is this function located
			// CBasePlayer* localPlayer = reinterpret_cast<CBasePlayer*>(interfaces::entityList->GetClientEntity(interfaces::engineClient->GetLocalPlayer()));
			// return cfw::vmt::get(localPlayer + 1);

			return (void*)cfw::findPattern("client.dll", "40 55 53 48 8D AC 24 ?? ?? ?? ?? B8 ?? ?? ?? ?? E8 ?? ?? ?? ?? 48 2B E0 48 8B 05 ?? ?? ?? ?? 48 33 C4 48 89 85 ?? ?? ?? ?? 48 89 BC 24 ?? ?? ?? ?? 41 8B D9");
		}

	public:
		SetupBonesHook(token) {}
	};
}

