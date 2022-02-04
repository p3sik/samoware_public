
#include "samoware/hooks/luastringdump.h"

#include "samoware/sdk/luajit.h"

namespace hooks {
	const char fakeBytecode[] = "nea";

	int StringDumpHookFunc(lua_State* state) {
		ILuaBase* luaBase = state->luabase;

		if (!luaBase->IsType(1, Lua::FUNCTION))
			goto callOriginal;

		luaBase->Push(1);

		lj_Debug dbg;
		if (!luajit::call_lua_getinfo(state, ">S", &dbg))
			goto callOriginal;

		if (!strcmp(dbg.source, "lua/includes/init.lua")) {
			cfw::Logger::Get().Log<cfw::LogLevel::WARNING>("Tried to use string.dump on protected function!");
			luaBase->PushString(fakeBytecode, sizeof(fakeBytecode));
			return 1;
		}

	callOriginal:

		return StringDumpHook::Get().GetOriginal()(state);
	}
}
