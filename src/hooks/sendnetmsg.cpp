
#include "samoware/hooks/sendnetmsg.h"

#include "samoware/hooks/clmove.h"
#include "samoware/sdk/cclientstate.h"
#include "samoware/sdk/chlclient.h"
#include "samoware/sdk/inetchannel.h"
#include "samoware/interfaces.h"
#include "samoware/globals.h"

#define MAX_CMD_BUFFER 4000

#define NUM_NEW_COMMAND_BITS		4
#define MAX_NEW_COMMANDS			((1 << NUM_NEW_COMMAND_BITS) - 1)

// Max number of history commands to send ( 2 by default ) in case of dropped packets
#define NUM_BACKUP_COMMAND_BITS		3
#define MAX_BACKUP_COMMANDS			((1 << NUM_BACKUP_COMMAND_BITS) - 1)

namespace hooks {
	void CL_SendMove() {
		auto& globals = Globals::Get();
		if (globals.tickbaseShiftRecharge > 0) {
			globals.tickbaseShiftRecharge--;
			globals.ticksAllowed++;
			return;
		}

		// Tickbase shift
		if (!globals.shiftingTickbase && globals.tickbaseShift > 0) {
			// choke current command
			interfaces::engineClient->GetNetChannel()->SetChoked();
			interfaces::clientState->chokedcommands++;

			int ticksAllowed = globals.ticksAllowed - interfaces::clientState->chokedcommands + 1;
			int shiftAmount = min(ticksAllowed, globals.tickbaseShift);

			globals.shiftingTickbase = true;
			for (int i = 0; i < shiftAmount; i++) {
				CL_MoveHook::Get().GetOriginal()(0.f, false);

				globals.ticksAllowed--;
				globals.tickbaseShift--;
			}
			globals.shiftingTickbase = false;

			// Send current & new ticks
		}

		uint8_t data[MAX_CMD_BUFFER];

		int nextCommandNr = interfaces::clientState->lastoutgoingcommand + interfaces::clientState->chokedcommands + 1;

		CLC_Move moveMsg;
		moveMsg.SetupVMT();

		moveMsg.m_DataOut.StartWriting(data, sizeof(data));

		// How many real new commands have queued up
		moveMsg.m_nNewCommands = 1 + interfaces::clientState->chokedcommands;
		moveMsg.m_nNewCommands = std::clamp(moveMsg.m_nNewCommands, 0, MAX_NEW_COMMANDS);

		int extraCommands = interfaces::clientState->chokedcommands + 1 - moveMsg.m_nNewCommands;

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
			INetChannel* chan = reinterpret_cast<INetChannel*>(interfaces::engineClient->GetNetChannelInfo());
	
			chan->m_nChokedPackets -= extraCommands;

			static float prevInterp = 0.f;
			float targetInterp = Globals::Get().targetInterp;
			if (Globals::Get().manipulateInterp && std::abs(targetInterp - prevInterp) > 0.001f) {
				bf_write& write = chan->m_StreamUnreliable;
				write.WriteUInt(static_cast<uint32_t>(NetMessage::net_SetConVar), 6);
				write.WriteByte(1);
				write.WriteString("cl_interp");

				char interpStr[64] {};
				snprintf(interpStr, sizeof(interpStr), "%.4f", targetInterp);
				write.WriteString(interpStr);

				prevInterp = targetInterp;
			}

			SendNetMsgHook::Get().GetOriginal()(chan, reinterpret_cast<INetMessage&>(moveMsg), false, false);
		}
	}

	bool __fastcall SendNetMsgHookFunc(INetChannel* self, INetMessage& msg, bool bForceReliable, bool bVoice) {
		// Fix voice while fake lagging
		if (msg.GetGroup() == static_cast<int>(NetMessage::clc_VoiceData))
			bVoice = true;
		
		auto msgName = msg.GetName();
		if (strcmp(msgName, "clc_Move") == 0) {
			CL_SendMove();
			return true;
		}

		return SendNetMsgHook::Get().GetOriginal()(self, msg, bForceReliable, bVoice);
	}
}
