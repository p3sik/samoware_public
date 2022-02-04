
#pragma once

#include "cfw/basefunchook.h"


#include "../sdk/iengineclient.h"
#include "../interfaces.h"

namespace hooks {
	bool __fastcall IsPlayingTimeDemoHookFunc(void* self);

	class IsPlayingTimeDemoHook : public cfw::BaseFunctionHook<IsPlayingTimeDemoHookFunc, cfw::HookType::VIRTUAL, IEngineClient::vIndex_IsPlayingTimeDemo>, public cfw::Singleton<IsPlayingTimeDemoHook> {
	private:
		const char* GetHookName() const { return "IsPlayingTimeDemo"; }

		void* GetHookTarget() {
			return cfw::vmt::get(interfaces::engineClient);
		}

		virtual void OnPreSetup() {
			isPlayingTimeDemoRet = (void*)(cfw::findPattern("client.dll", "48 8B 05 ?? ?? ?? ?? 48 8D 0D ?? ?? ?? ?? FF 50 68 48 8B 0D ?? ?? ?? ?? 85 C0 0F 95 05 ?? ?? ?? ?? 48 8B 01 FF 90 70 02 00 00") + 42);
		}

	public:
		void* isPlayingTimeDemoRet;

		IsPlayingTimeDemoHook(token) : isPlayingTimeDemoRet(nullptr) {}
	};
}

