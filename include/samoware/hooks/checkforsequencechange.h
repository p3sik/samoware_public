
#pragma once

#include "cfw/basefunchook.h"

class CStudioHdr;

namespace hooks {
	bool __fastcall CheckForSequenceChangeHookFunc(void* self, CStudioHdr* hdr, int nCurSequence, bool bForceNewSequence, bool bInterpolate);

	class CheckForSequenceChangeHook : public cfw::BaseFunctionHook<CheckForSequenceChangeHookFunc, cfw::HookType::STANDARD, 0>, public cfw::Singleton<CheckForSequenceChangeHook> {
	private:
		const char* GetHookName() const { return "CheckForSequenceChange"; }

		void* GetHookTarget() {
			return (void*)cfw::findPattern("client.dll", "48 85 D2 0F 84 D5 00 00 00 48 89 6C 24 10 48 89 74 24 18");
		}

	public:
		CheckForSequenceChangeHook(token) {}
	};
}

