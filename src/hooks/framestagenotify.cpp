
#include "samoware/hooks/framestagenotify.h"

#include "samoware/sdk/bitbuf.h"
#include "samoware/sdk/cprediction.h"
#include "samoware/sdk/iengineclient.h"
#include "samoware/sdk/icliententitylist.h"
#include "samoware/sdk/inetchannel.h"
#include "samoware/sdk/cglobalvars.h"

#include "samoware/hooks/updateclientsideanimation.h"

#include "samoware/cheats/misc.h"
#include "samoware/cheats/luaapi.h"

#include "samoware/config.h"
#include "samoware/globals.h"

namespace hooks {
	void __fastcall FrameStageNotifyHookFunc(CHLClient* self, ClientFrameStage_t stage) {
		if (stage == ClientFrameStage_t::FRAME_START) {
			if (interfaces::engineClient->IsInGame()) {
				if (!interfaces::runtimeSetup)
					interfaces::setupRuntime();
			} else {
				interfaces::runtimeSetup = false;
				interfaces::clientLua = nullptr;
			}
		}

		if (stage == ClientFrameStage_t::FRAME_RENDER_START) {
			static int prevTick = 0;
			int currentTick = interfaces::globalVars->tickcount;

			Globals::Get().isNewTick = currentTick != prevTick;

			if (currentTick != prevTick) {
				Misc::Get().Lagger();
				prevTick = currentTick;
			}
		}

		if (stage == ClientFrameStage_t::FRAME_RENDER_START)
			interfaces::prediction->SetLocalViewAngles(Globals::Get().currentViewAngles);

		luaapi::CallHook("PreFrameStageNotify", 1, [&](ILuaBase* LUA) {
			LUA->PushNumber(static_cast<int>(stage));
		});

		FrameStageNotifyHook::Get().GetOriginal()(self, stage);

		luaapi::CallHook("PostFrameStageNotify", 1, [&](ILuaBase* LUA) {
			LUA->PushNumber(static_cast<int>(stage));
		 });
	}
}
