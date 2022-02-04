
#pragma once

#include "cfw/basefunchook.h"

#include "../sdk/iengineclient.h"
#include "../sdk/icliententitylist.h"
#include "../sdk/cbaseentity.h"
#include "../interfaces.h"

namespace hooks {
	void __fastcall UpdateClientsideAnimationHookFunc(CBasePlayer* self);

	class UpdateClientsideAnimationHook : public cfw::BaseFunctionHook<UpdateClientsideAnimationHookFunc, cfw::HookType::VIRTUAL, CBasePlayer::vIndex_UpdateClientsideAnimation>, public cfw::Singleton<UpdateClientsideAnimationHook> {
	private:
		const char* GetHookName() const { return "UpdateClientsideAnimation"; }

		void* GetHookTarget() {
			CBasePlayer* localPlayer = reinterpret_cast<CBasePlayer*>(interfaces::entityList->GetClientEntity(interfaces::engineClient->GetLocalPlayer()));
			return cfw::vmt::get(localPlayer);
		}

	public:
		UpdateClientsideAnimationHook(token) {}
	};
}
