
#include <assert.h>

#include "cfw/logger.h"

#include "samoware/cheats/lualoader.h"
#include "samoware/cheats/luaapi.h"

#include "samoware/sdk/iengineclient.h"
#include "samoware/sdk/iluabase.h"
#include "samoware/sdk/luajit.h"

#include "samoware/cheat.h"

LuaLoader::LuaLoader(token) {

}

const char luaLoaderHeader[] = "local me = LocalPlayer();local this = me:GetEyeTrace().Entity;local there = me:GetEyeTrace().HitPos;";

void LuaLoader::ProcessQueue() {
	if (!interfaces::engineClient->IsInGame()) {
		if (!_queue.empty()) {
			while (!_queue.empty()) {
				_queue.pop();
			}
		}

		return;
	}

	if (!interfaces::clientLua)
		return;

	_mutex.lock();

	ILuaBase* lua = interfaces::clientLua;
	lua_State* state = lua->GetLuaState();

	while (!_queue.empty()) {
		const std::string& script = _queue.top();

		std::string toRun = luaLoaderHeader + script;

		luajit::call_luaL_loadbufferx(state, toRun.c_str(), toRun.length(), "lua/includes/init.lua", NULL);
		if (lua->IsType(-1, Lua::FUNCTION)) {
			luaapi::PushLuaApi();

			if (lua->PCall(0, 0, 0)) {
				std::string errorMessage = lua->GetString(-1);

				cfw::Logger::Get().Log<cfw::LogLevel::ERROR>("Lua pcall error: ", errorMessage.c_str());

				// Pop error
				lua->Pop();
			}
		} else {
			std::string errorMessage = lua->GetString(-1);
			cfw::Logger::Get().Log<cfw::LogLevel::ERROR>("Failed to run script: ", errorMessage.c_str());

			std::cmatch match;
			static const std::regex errorLineRegex("\"\\]:(\\d+):");
			std::regex_search(errorMessage.c_str(), match, errorLineRegex);

			if (match.size() > 1) {
				int errorLine = std::stoi(match[1].str());
				
				auto& markers = Samoware::Get().menu->GetTextEditor().GetErrorMarkers();
				markers.clear();

				markers[errorLine] = errorMessage;
			}

			// Pop error message
			lua->Pop();
		}

		_queue.pop();
	}

	_mutex.unlock();
}

void LuaLoader::AddToQueue(const std::string& script) {
	_mutex.lock();
	_queue.push(script);
	_mutex.unlock();
}
