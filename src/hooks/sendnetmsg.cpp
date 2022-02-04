
#include "samoware/hooks/sendnetmsg.h"

namespace hooks {
	bool __fastcall SendNetMsgHookFunc(INetChannel* self, INetMessage& msg, bool bForceReliable, bool bVoice) {
		// Fix voice while fake lagging ((doesn't work actually))
		if (msg.GetGroup() == static_cast<int>(NetMessage::clc_VoiceData))
			bVoice = true;

		return SendNetMsgHook::Get().GetOriginal()(self, msg, bForceReliable, bVoice);
	}
}
