
#include "samoware/hooks/createmove.h"

#include <algorithm>

#include "samoware/sdk/cusercmd.h"
#include "samoware/sdk/cinput.h"
#include "samoware/sdk/angle.h"
#include "samoware/sdk/iengineclient.h"
#include "samoware/sdk/bitbuf.h"
#include "samoware/sdk/inetchannel.h"

#include "samoware/cheats/misc.h"
#include "samoware/cheats/luaapi.h"
#include "samoware/cheats/prediction.h"

#include "samoware/hooks/shouldinterpolate.h"
#include "samoware/hooks/updateclientsideanimation.h"
#include "samoware/hooks/sendnetmsg.h"

#include "samoware/interfaces.h"
#include "samoware/external.h"
#include "samoware/globals.h"
#include "samoware/config.h"

namespace hooks {
	void __fastcall CreateMoveHookFunc(CHLClient* self, int sequence_number, float input_sample_frametime, bool active) {
		static bool runtimeHooksInit = false;
		static void* jePtr;
		if (!runtimeHooksInit) {
			runtimeHooksInit = true;

			ShouldInterpolateHook::Get().Setup();
			UpdateClientsideAnimationHook::Get().Setup();
			SendNetMsgHook::Get().Setup();

			std::uintptr_t clMovePtr = cfw::findPattern("engine.dll", "40 55 53 48 8D AC 24 38 F0 FF FF B8 C8 10 00 00 E8 ?? ?? ?? ?? 48 2B E0 0F 29 B4 24 B0 10 00 00");
			jePtr = reinterpret_cast<void*>(clMovePtr + 0x16a);

			DWORD dummy;
			VirtualProtect(jePtr, 8, PAGE_EXECUTE_READWRITE, &dummy);
		}

		CreateMoveHook::Get().GetOriginal()(self, sequence_number, input_sample_frametime, active);

		CVerifiedUserCmd* vcmd = interfaces::input->GetVerifiedCommand(sequence_number);
		CUserCmd* cmd = &vcmd->cmd;

		luaapi::CallHook("PostCreateMove", 1, [&](ILuaBase* LUA) -> void {
			LUA->PushUserType(cmd, Lua::USERCMD);
		});

		if (!cmd || !cmd->command_number)
			return;

		cmd->forwardmove = std::clamp(cmd->forwardmove, -10000.f, 10000.f);
		cmd->sidemove = std::clamp(cmd->sidemove, -10000.f, 10000.f);
		cmd->upmove = std::clamp(cmd->upmove, -10000.f, 10000.f);

		/*
		luaapi::CallHook("OverrideCreateMove", 1, [&](ILuaBase* LUA) -> void {
			LUA->PushUserType(cmd, Lua::USERCMD);
						 });
		*/

		{
			Angle& prevViewAngles = CreateMoveHook::Get().prevViewAngles;

			// m_pitch
			Angle diff = cmd->viewangles - prevViewAngles;
			diff.Normalize();

			diff /= 0.022f * 1.28f;

			cmd->mousedx = static_cast<short>(diff.y);
			cmd->mousedy = static_cast<short>(diff.p);

			prevViewAngles = cmd->viewangles;
		}

		Globals::Get().currentViewAngles = cmd->viewangles;

		Globals& globals = Globals::Get();
		// external::setR14b(!globals.forceChoke && !globals.shouldChoke);
		if (!globals.forceChoke && !globals.shouldChoke) {
			// bSendPacket = true
			static uint8_t jeBytes[] = {0x0f, 0x84, 0xBD, 0x02, 0x00, 0x00};
			memcpy(jePtr, jeBytes, 6);
		} else {
			// bSendPacket = false
			static uint8_t jmpBytes[] = {0xe9, 0xbe, 0x02, 0x00, 0x00, 0x90};
			memcpy(jePtr, jmpBytes, 6);
		}

		/*auto& out = cmds.emplace_back();

		out.is_outgoing = (!globals.forceChoke && !globals.shouldChoke);
		out.is_used = false;
		out.command_number = cmd->command_number;
		out.previous_command_number = 0;

		while (cmds.size() > (int)(1.0f / interfaces::globalVars->interval_per_tick))
			cmds.pop_front();

		if (globals.forceChoke || globals.shouldChoke) {
			INetChannel* netChan = interfaces::engineClient->GetNetChannel();

			if (netChan->m_nChokedPackets > 0 && !(netChan->m_nChokedPackets % 4)) {
				auto backup_choke = netChan->m_nChokedPackets;
				netChan->m_nChokedPackets = 0;

				netChan->SendDatagram(NULL);

				netChan->m_nOutSequenceNr--;
				netChan->m_nChokedPackets = backup_choke;
			}
		}*/

		vcmd->crc = cmd->GetChecksum();
	}
}
