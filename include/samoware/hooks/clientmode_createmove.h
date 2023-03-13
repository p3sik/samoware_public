
#pragma once

#include "cfw/basefunchook.h"

#include "../sdk/clientmodeshared.h"
#include "../sdk/angle.h"
#include "../interfaces.h"

namespace hooks {
bool __fastcall ClientModeCreateMoveHookFunc(ClientModeShared* self, float flInputSampleTime, CUserCmd* cmd);

class ClientModeCreateMoveHook : public cfw::BaseFunctionHook<ClientModeCreateMoveHookFunc, cfw::HookType::VIRTUAL, ClientModeShared::vIndex_CreateMove>, public cfw::Singleton<ClientModeCreateMoveHook> {
public:
	Angle prevViewAngles;

private:
	const char* GetHookName() const { return "ClientModeCreateMove"; }

	void* GetHookTarget() {
		return cfw::vmt::get(interfaces::clientMode);
	}

public:
	ClientModeCreateMoveHook(token) {}
};
}
