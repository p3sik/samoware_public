
#pragma once

#include "cfw/singleton.h"
#include "cfw/util.h"

#include "iluabase.h"

class CBasePlayer;

#define LUA_IDSIZE 60

struct lj_Debug {
	/* Common fields. Must be in the same order as in lua.h. */
	int event;
	const char* name;
	const char* namewhat;
	const char* what;
	const char* source;
	int currentline;
	int nups;
	int linedefined;
	int lastlinedefined;
	char short_src[LUA_IDSIZE];
	int i_ci;
	/* Extended fields. Only valid if lj_debug_getinfo() is called with ext = 1.*/
	int nparams;
	int isvararg;
};

#define SIGFUNC(sig, ret, name, args, ...)	\
	static std::uintptr_t find_##name() {	\
		static std::uintptr_t funcAddr = cfw::findPattern("lua_shared.dll", sig);	\
		return funcAddr;					\
	}										\
	static ret call_##name args {			\
		return reinterpret_cast<ret(__cdecl*)args>(find_##name())(__VA_ARGS__);	\
	}

#define IMPORTFUNC(ret, name, args, ...)	\
	static std::uintptr_t find_##name() {	\
		static std::uintptr_t funcAddr = reinterpret_cast<std::uintptr_t>(GetProcAddress(GetModuleHandleA("lua_shared.dll"), #name));	\
		return funcAddr;					\
	}										\
	static ret call_##name args {			\
		return reinterpret_cast<ret(__cdecl*)args>(find_##name())(__VA_ARGS__);	\
	}

namespace luajit {
	SIGFUNC("48 89 5C 24 08 57 48 83 EC 30 BA 01 00 00 00 48 8B D9 E8", int, string_dump, (lua_State* state), state);

	IMPORTFUNC(int, lua_setfenv, (lua_State* state, int pos), state, pos);
	IMPORTFUNC(int, luaL_loadbufferx, (lua_State* state, const char* buf, std::size_t size, const char* name, const char* mode), state, buf, size, name, mode);
	IMPORTFUNC(int, lua_getinfo, (lua_State* state, const char* what, lj_Debug* ar), state, what, ar);

	typedef bool(__fastcall* CLuaGamemode__CallWithArgsFn)(CLuaGamemode* self, int hookNum);
	typedef bool(__fastcall* CLuaGamemode__CallFinishFn)(CLuaGamemode* self, int unk);
	typedef void(__fastcall* LuaHook_UpdateAnimationFn)(CBasePlayer* self);

	extern CLuaGamemode* luaGamemode;
	extern CLuaGamemode__CallWithArgsFn CLuaGamemode__CallWithArgs;
	extern CLuaGamemode__CallFinishFn CLuaGamemode__CallFinish;
	extern LuaHook_UpdateAnimationFn luaHook_UpdateAnimation;

	void setup();
}
