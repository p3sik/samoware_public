
#pragma once

class CHLClient;
class bf_write;
class INetChannel;
class INetMessage;

namespace hooks {
	namespace sendnetmsg {
		typedef bool(__thiscall* SendNetMsgFn)(INetChannel*, INetMessage&, bool, bool);
		extern SendNetMsgFn SendNetMsgOrig;

		extern bool __fastcall SendNetMsgHook(INetChannel* self, INetMessage& msg, bool bForceReliable, bool bVoice);

		extern bool hook();
		extern bool unhook();
	}
}
