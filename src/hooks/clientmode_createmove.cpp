
#include "samoware/hooks/clientmode_createmove.h"

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

bool __fastcall ClientModeCreateMoveHookFunc(ClientModeShared* self, float flInputSampleTime, CUserCmd* cmd) {
	luaapi::CallHook("PreCreateMove", 1, [&](ILuaBase* LUA) -> void {
		LUA->PushUserType(cmd, Lua::USERCMD);
	});

	if (!cmd->command_number)
		return ClientModeCreateMoveHook::Get().GetOriginal()(self, flInputSampleTime, cmd);

	// Update server time
	EnginePrediction::Get().GetServerTime(cmd);

	Globals& globals = Globals::Get();
	globals.forceChoke = false;
	globals.shouldChoke = false;

	Misc& misc = Misc::Get();

	misc.ChangeName();

	misc.UseSpam(cmd);
	misc.FlashlightSpam(cmd);
	misc.ArmBreaker(cmd);
	misc.FastWalk(cmd);
	misc.AutoStrafe(cmd);
	misc.BunnyHop(cmd);

	return ClientModeCreateMoveHook::Get().GetOriginal()(self, flInputSampleTime, cmd);
}

}
