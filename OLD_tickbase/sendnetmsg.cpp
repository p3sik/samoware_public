
#include <hooks/sendnetmsg.h>
#include <hooks/clmove.h>

#include <sdk/iengineclient.h>
#include <sdk/inetchannel.h>
#include <sdk/cclientstate.h>
#include <sdk/chlclient.h>
#include <sdk/bitbuf.h>

#include <interfaces.h>
#include <vmthook.h>
#include <fnv.h>

#include <algorithm>

#define MAX_CMD_BUFFER 4000

#define NUM_NEW_COMMAND_BITS		4
#define MAX_NEW_COMMANDS			((1 << NUM_NEW_COMMAND_BITS) - 1)

// Max number of history commands to send ( 2 by default ) in case of dropped packets
#define NUM_BACKUP_COMMAND_BITS		3
#define MAX_BACKUP_COMMANDS			((1 << NUM_BACKUP_COMMAND_BITS) - 1)

void CL_SendMove();

namespace hooks {
	namespace sendnetmsg {
		SendNetMsgFn SendNetMsgOrig = nullptr;

		bool __fastcall SendNetMsgHook(INetChannel* self, INetMessage& msg, bool bForceReliable, bool bVoice) {
			if (fnv::hashRuntime(msg.GetName()) == fnv::hash("clc_Move")) {
				CL_SendMove();
				return true;
			}

			return SendNetMsgOrig(self, msg, bForceReliable, bVoice);
		}

		static void** netChanVmt = nullptr;
		bool hook() {
			netChanVmt = vmt::get(interfaces::engineclient->GetNetChannelInfo());

			return vmt::hook_vmt<SendNetMsgFn>(netChanVmt, &SendNetMsgOrig, SendNetMsgHook, 40);
		}

		bool unhook() {
			if (!netChanVmt)
				return true;

			return vmt::hook_vmt<SendNetMsgFn>(netChanVmt, nullptr, SendNetMsgOrig, 40);
		}
	}
}

void CL_SendMove() {
	if (globals::ticksAllowed < globals::sv_maxusrcmdprocessticks && (globals::rechargingShift || globals::rechargingTick)) {
		globals::ticksAllowed++;
		globals::rechargingTick = false;
		return;
	}

	// tickbase shift
	if (!globals::shiftingTickbase && globals::ticksAllowed > 0 && globals::tickbaseShift > 0) {
		globals::ticksAllowed = min(globals::ticksAllowed, globals::sv_maxusrcmdprocessticks - 1);

		// choke current command
		INetChannel* chan = reinterpret_cast<INetChannel*>(interfaces::engineclient->GetNetChannelInfo());
		chan->SetChoked();

		interfaces::clientstate->chokedcommands++;
		globals::ticksAllowed--;

		globals::shiftingTickbase = true;

		int ticksAllowed = globals::ticksAllowed - interfaces::clientstate->chokedcommands + 1;
		int shiftAmount = min(ticksAllowed, globals::tickbaseShift);
		for (int i = 0; i < shiftAmount; i++) {
			hooks::cl_move::CL_MoveOrig(0.f, true);
			globals::ticksAllowed--;
		}

		globals::tickbaseShift = 0;
		globals::shiftingTickbase = false;

		// Send new ticks
	}

	byte data[MAX_CMD_BUFFER];

	int nextCommandNr = interfaces::clientstate->lastoutgoingcommand + interfaces::clientstate->chokedcommands + 1;

	CLC_Move moveMsg;
	moveMsg.SetupVMT();

	moveMsg.m_DataOut.StartWriting(data, sizeof(data));

	// How many real new commands have queued up
	moveMsg.m_nNewCommands = 1 + interfaces::clientstate->chokedcommands;
	moveMsg.m_nNewCommands = std::clamp(moveMsg.m_nNewCommands, 0, MAX_NEW_COMMANDS);

	int extraCommands = interfaces::clientstate->chokedcommands + 1 - moveMsg.m_nNewCommands;

	int backupCommands = max(2, extraCommands);
	moveMsg.m_nBackupCommands = std::clamp(backupCommands, 0, MAX_BACKUP_COMMANDS);

	int numCommands = moveMsg.m_nNewCommands + moveMsg.m_nBackupCommands;

	int from = -1;	// first command is deltaed against zeros 

	bool bOK = true;

	for (int to = nextCommandNr - numCommands + 1; to <= nextCommandNr; to++) {
		bool isnewcmd = to >= (nextCommandNr - moveMsg.m_nNewCommands + 1);

		// first valid command number is 1
		bOK = bOK && interfaces::client->WriteUsercmdDeltaToBuffer(&moveMsg.m_DataOut, from, to, isnewcmd);
		from = to;
	}

	// only write message if all usercmds were written correctly, otherwise parsing would fail
	if (bOK) {
		INetChannel* chan = reinterpret_cast<INetChannel*>(interfaces::engineclient->GetNetChannelInfo());

		chan->m_nChokedPackets -= extraCommands;

		hooks::sendnetmsg::SendNetMsgOrig(chan, reinterpret_cast<INetMessage&>(moveMsg), false, false);
	}
}
