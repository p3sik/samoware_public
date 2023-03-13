
#include "samoware/hooks/clmove.h"

#include "samoware/globals.h"

#include "samoware/cheats/misc.h"
namespace hooks {

void __fastcall CL_MoveHookFunc(float accumulated_extra_samples, bool bFinalTick) {
	auto& globals = Globals::Get();
	if (globals.tickbaseShiftRecharge > 0) {
		globals.tickbaseShiftRecharge--;
		globals.ticksAllowed++;
		return;
	}

	CL_MoveHook::Get().GetOriginal()(accumulated_extra_samples, bFinalTick);
}

}
