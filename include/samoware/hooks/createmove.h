
#pragma once

#include "cfw/basefunchook.h"

#include "../sdk/chlclient.h"
#include "../sdk/angle.h"
#include "../interfaces.h"

class CHLClient;

namespace hooks {
	void __fastcall CreateMoveHookFunc(CHLClient* self, int sequence_number, float input_sample_frametime, bool active);

	class CreateMoveHook : public cfw::BaseFunctionHook<CreateMoveHookFunc, cfw::HookType::VIRTUAL, CHLClient::vIndex_CreateMove>, public cfw::Singleton<CreateMoveHook> {
	public:
		Angle prevViewAngles;

	private:
		const char* GetHookName() const { return "CreateMove"; }

		void* GetHookTarget() {
			return cfw::vmt::get(interfaces::client);
		}

	public:
		CreateMoveHook(token) {}
	};
}
