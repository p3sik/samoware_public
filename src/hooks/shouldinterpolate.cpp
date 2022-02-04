
#include "samoware/hooks/shouldinterpolate.h"

#include "samoware/config.h"

namespace hooks {
	bool __fastcall ShouldInterpolateHookFunc(CBaseEntity* self) {
		if (!Config::Get().hvh.disableInterpolation)
			return ShouldInterpolateHook::Get().GetOriginal()(self);

		CBaseEntity* localPlayer = interfaces::entityList->GetClientEntity(interfaces::engineClient->GetLocalPlayer());
		return (self == localPlayer) ? ShouldInterpolateHook::Get().GetOriginal()(self) : false;
	}
}
