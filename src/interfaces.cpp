
#include "samoware/interfaces.h"

#include "samoware/sdk/chlclient.h"
#include "samoware/sdk/iluabase.h"
#include "samoware/sdk/cluashared.h"
#include "samoware/sdk/cclientstate.h"

#include "cfw/logger.h"
#include "cfw/util.h"

#include <Windows.h>

typedef void* (__cdecl* CreateInterfaceFn)(const char*, int*);

namespace interfaces {
	enum IFaceLocation {
		CLIENT,
		ENGINE,
		LUASHARED,
		VGUI
	};

	CreateInterfaceFn ClientCreateInterface = nullptr;
	CreateInterfaceFn EngineCreateInterface = nullptr;
	CreateInterfaceFn LuaSharedCreateInterface = nullptr;
	CreateInterfaceFn VGUICreateInterface = nullptr;

	bool hadError = false;

	template <IFaceLocation location, typename T>
	static void CreateInterface(const char* name, T*& outIFace) {
		int status;

		void* iface;
		if constexpr (location == CLIENT) {
			iface = ClientCreateInterface(name, &status);
		} else if constexpr (location == ENGINE) {
			iface = EngineCreateInterface(name, &status);
		} else if constexpr (location == LUASHARED) {
			iface = LuaSharedCreateInterface(name, &status);
		} else {
			iface = VGUICreateInterface(name, &status);
		}

		if (!iface) {
			cfw::Logger::Get().Log<cfw::LogLevel::ERROR>("Failed to get interface ", name);
			hadError = true;
		}

		cfw::Logger::Get().Log<cfw::LogLevel::DEBUG>(name, ": 0x", std::hex, iface);

		outIFace = reinterpret_cast<T*>(iface);
	}

	IEngineClient* engineClient = nullptr;
	CHLClient* client = nullptr;
	CInput* input = nullptr;
	CClientState* clientState = nullptr;
	ClientModeShared* clientMode = nullptr;
	CGlobalVars* globalVars = nullptr;
	IClientEntityList* entityList = nullptr;
	IPanel* panel = nullptr;
	CPrediction* prediction = nullptr;
	IGameMovement* gameMovement = nullptr;
	IMoveHelper* moveHelper = nullptr;
	IEngineTrace* engineTrace = nullptr;
	CLuaShared* luaShared = nullptr;
	ILuaBase* clientLua = nullptr;

	bool runtimeSetup = false;

	bool setup() {
		HMODULE clientDll = GetModuleHandleA("client.dll");
		HMODULE engineDll = GetModuleHandleA("engine.dll");
		HMODULE luaSharedDll = GetModuleHandleA("lua_shared.dll");
		HMODULE vguiDll = GetModuleHandleA("vgui2.dll");

		ClientCreateInterface = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(clientDll, "CreateInterface"));
		EngineCreateInterface = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(engineDll, "CreateInterface"));
		LuaSharedCreateInterface = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(luaSharedDll, "CreateInterface"));
		VGUICreateInterface = reinterpret_cast<CreateInterfaceFn>(GetProcAddress(vguiDll, "CreateInterface"));

		CreateInterface<VGUI>("VGUI_Panel009", panel);
		CreateInterface<LUASHARED>("LUASHARED003", luaShared);
		CreateInterface<ENGINE>("VEngineClient015", engineClient);
		CreateInterface<CLIENT>("VClient017", client);
		CreateInterface<CLIENT>("VClientEntityList003", entityList);
		CreateInterface<CLIENT>("VClientPrediction001", prediction);
		CreateInterface<CLIENT>("GameMovement001", gameMovement);
		CreateInterface<ENGINE>("EngineTraceClient003", engineTrace);

		if (client) {
			std::uintptr_t createMovePtr = cfw::vmt::get<std::uintptr_t>(client, CHLClient::vIndex_CreateMove);
			input = *reinterpret_cast<CInput**>(cfw::getAbsAddr(createMovePtr + 0x3F));

			cfw::Logger::Get().Log<cfw::LogLevel::DEBUG>("CInput: 0x", std::hex, input);

			std::uintptr_t initPtr = cfw::vmt::get<std::uintptr_t>(client, 0);
			globalVars = *reinterpret_cast<CGlobalVars**>(cfw::getAbsAddr(initPtr + 0x94));

			cfw::Logger::Get().Log<cfw::LogLevel::DEBUG>("CGlobalVars: 0x", std::hex, globalVars);
		}

		std::uintptr_t moveHelperPtr = cfw::findPattern("client.dll", "48 89 78 68 89 78 70 48 8B 43 10 48 89 78 74 89 78 7C 48 8B 0D ") + 21;
		if (moveHelperPtr) {
			moveHelper = reinterpret_cast<IMoveHelper*>(cfw::getAbsAddr(moveHelperPtr));

			cfw::Logger::Get().Log<cfw::LogLevel::DEBUG>("IMoveHelper: 0x", std::hex, moveHelper);
		}

		std::uintptr_t clMovePtr = cfw::findPattern("engine.dll", "40 55 53 48 8D AC 24 38 F0 FF FF B8 C8 10 00 00 E8 ?? ?? ?? ?? 48 2B E0 0F 29 B4 24 B0 10 00 00");
		if (clMovePtr) {
			void* chokedCommandsPtr = cfw::getAbsAddr(clMovePtr + 0x110, 2);
			clientState = reinterpret_cast<CClientState*>(reinterpret_cast<std::uintptr_t>(chokedCommandsPtr) - offsetof(CClientState, chokedcommands) - 1);

			cfw::Logger::Get().Log<cfw::LogLevel::DEBUG>("CClientState: 0x", std::hex, clientState);
		}

		std::uintptr_t hudProcessInput = cfw::vmt::get<std::uintptr_t>(client, 10);
		if (hudProcessInput) {
			clientMode = *reinterpret_cast<ClientModeShared**>(cfw::getAbsAddr(hudProcessInput));
			
			cfw::Logger::Get().Log<cfw::LogLevel::DEBUG>("ClientModeShared: 0x", std::hex, clientMode);
		}

		return hadError;
	}

	bool setupRuntime() {
		runtimeSetup = true;

		clientLua = luaShared->GetLuaInterface(Lua::CLIENT);
		cfw::Logger::Get().Log<cfw::LogLevel::DEBUG>("clientLua: 0x", std::hex, clientLua);
		if (!clientLua)
			return true;

		return false;
	}
}
