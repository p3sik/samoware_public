
#include "samoware/hooks/isplayingtimedemo.h"

#include "samoware/config.h"

#include <intrin.h>
#pragma intrinsic(_ReturnAddress)

namespace hooks {
	bool __fastcall IsPlayingTimeDemoHookFunc(void* self) {
		if (Config::Get().hvh.disableInterpolation && _ReturnAddress() == IsPlayingTimeDemoHook::Get().isPlayingTimeDemoRet)
			return true;

		return IsPlayingTimeDemoHook::Get().GetOriginal()(self);
	}
}
