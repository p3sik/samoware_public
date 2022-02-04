
#include "samoware/hooks/checkforsequencechange.h"

#include "samoware/config.h"

namespace hooks {
	bool __fastcall CheckForSequenceChangeHookFunc(void* self, CStudioHdr* hdr, int nCurSequence, bool bForceNewSequence, bool bInterpolate) {
		if (Config::Get().hvh.disableSequenceInterpolation)
			bInterpolate = false;

		return CheckForSequenceChangeHook::Get().GetOriginal()(self, hdr, nCurSequence, bForceNewSequence, bInterpolate);
	}
}
