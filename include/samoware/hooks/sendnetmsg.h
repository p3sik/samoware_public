
#pragma once

#include "cfw/basefunchook.h"

#include "../sdk/inetchannel.h"
#include "../sdk/iengineclient.h"
#include "../interfaces.h"

namespace hooks {
	bool __fastcall SendNetMsgHookFunc(INetChannel* self, INetMessage& msg, bool bForceReliable = false, bool bVoice = false);

	class SendNetMsgHook : public cfw::BaseFunctionHook<SendNetMsgHookFunc, cfw::HookType::VIRTUAL, INetChannel::vIndex_SendNetMsg>, public cfw::Singleton<SendNetMsgHook> {
	private:
		const char* GetHookName() const { return "SendNetMsg"; }

		void* GetHookTarget() {
			return cfw::vmt::get(interfaces::engineClient->GetNetChannel());
		}

	public:
		SendNetMsgHook(token) {}
	};
}
