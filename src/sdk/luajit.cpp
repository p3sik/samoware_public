
#include "samoware/sdk/luajit.h"

namespace luajit {
	CLuaGamemode* luaGamemode = nullptr;
	CLuaGamemode__CallWithArgsFn CLuaGamemode__CallWithArgs = nullptr;
	CLuaGamemode__CallFinishFn CLuaGamemode__CallFinish = nullptr;
	LuaHook_UpdateAnimationFn luaHook_UpdateAnimation = nullptr;

	void setup() {
		if (luaGamemode)
			return;

		luaGamemode = reinterpret_cast<CLuaGamemode*>(cfw::getAbsAddr(cfw::findPattern("client.dll", "40 53 48 83 EC 30 48 8B D9 8B 89 C0 38 00 00 83 F9 FF 74 63") + 0x37));
		CLuaGamemode__CallWithArgs = reinterpret_cast<CLuaGamemode__CallWithArgsFn>(cfw::findPattern("client.dll", "48 8B 03 48 8B CB FF 90 18 01 00 00 84 C0 75 04 32 DB EB 2E") - 0x75);
		CLuaGamemode__CallFinish = reinterpret_cast<CLuaGamemode__CallFinishFn>(cfw::findPattern("client.dll", "48 8B 0D ?? ?? ?? ?? 8D 53 02 48 8B 01 FF 90 10 02 00 00 0F B6 D8 85 FF") - 0x41);
		luaHook_UpdateAnimation = reinterpret_cast<LuaHook_UpdateAnimationFn>(cfw::findPattern("client.dll", "48 83 EC 48 48 83 3D 5C ?? ?? ?? ?? 74 76 48 85 C9 74 0E 48 8B 01 FF 90 60 05 00 00"));
	}
}
