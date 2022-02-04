
#pragma once

#include "cfw/basefunchook.h"

#include "../sdk/cprediction.h"
#include "../interfaces.h"

namespace hooks {
	void __fastcall RunCommandHookFunc(CPrediction* self, CBaseEntity* player, CUserCmd* ucmd, IMoveHelper* moveHelper);

	class RunCommandHook : public cfw::BaseFunctionHook<RunCommandHookFunc, cfw::HookType::VIRTUAL, CPrediction::vIndex_RunCommand>, public cfw::Singleton<RunCommandHook> {
	private:
		const char* GetHookName() const { return "RunCommand"; }

		void* GetHookTarget() {
			return cfw::vmt::get(interfaces::prediction);
		}

	public:
		RunCommandHook(token) {}
	};
}
