
#include "samoware/hooks/luastringdump.h"

#include "samoware/sdk/luajit.h"

namespace hooks {
	const char fakeBytecode[] = "\x1BLJ\x2\x8\xB@lua/ok.luab\x0\x0\x3\x0\x2\x0\x4\x5\x2\x06\x0\x0\x0'\x2\x1\x0B\x0\x2\x1K\x0\x1\x0Awhat the fuck? fuck off retard https://discord.gg/xxxxxxxxxx\xAprint\x0\x0\x0\x0\x0\x0";

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
