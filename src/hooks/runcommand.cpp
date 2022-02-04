
#include "samoware/hooks/runcommand.h"

#include "samoware/sdk/cbaseentity.h"
#include "samoware/sdk/cusercmd.h"

#include "samoware/cheats/luaapi.h"

namespace hooks {
	void __fastcall RunCommandHookFunc(CPrediction* self, CBaseEntity* player, CUserCmd* ucmd, IMoveHelper* moveHelper) {
		RunCommandHook::Get().GetOriginal()(self, player, ucmd, moveHelper);
	}
}
