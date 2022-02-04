
#pragma once

#include "cfw/basefunchook.h"

#include "../sdk/iengineclient.h"
#include "../sdk/icliententitylist.h"
#include "../sdk/cbaseentity.h"
#include "../interfaces.h"

class matrix3x4_t;

namespace hooks {
	bool __fastcall ShouldInterpolateHookFunc(CBaseEntity* self);

	class ShouldInterpolateHook : public cfw::BaseFunctionHook<ShouldInterpolateHookFunc, cfw::HookType::VIRTUAL, CBaseEntity::vIndex_ShouldInterpolate>, public cfw::Singleton<ShouldInterpolateHook> {
	private:
		const char* GetHookName() const { return "ShouldInterpolate"; }

		void* GetHookTarget() {
			CBasePlayer* localPlayer = reinterpret_cast<CBasePlayer*>(interfaces::entityList->GetClientEntity(interfaces::engineClient->GetLocalPlayer()));
			return cfw::vmt::get(localPlayer);
		}

	public:
		ShouldInterpolateHook(token) {}
	};
}

