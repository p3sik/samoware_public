
#pragma once

#include "cfw/basefunchook.h"
#include "cfw/util.h"

#include "../sdk/iluabase.h"
#include "../sdk/luajit.h"
#include "../interfaces.h"

class lua_State;

namespace hooks {
	int __cdecl StringDumpHookFunc(lua_State* state);

	class StringDumpHook : public cfw::BaseFunctionHook<StringDumpHookFunc, cfw::HookType::STANDARD, 0>, public cfw::Singleton<StringDumpHook> {
	private:
		const char* GetHookName() const { return "string_dump"; }

		void* GetHookTarget() {
			return reinterpret_cast<void*>(luajit::find_string_dump());
		}

	public:
		StringDumpHook(token) {}
	};
}
