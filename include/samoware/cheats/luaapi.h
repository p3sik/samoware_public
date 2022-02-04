
#pragma once

#include <unordered_map>
#include <memory>
#include <functional>

#include "samoware/sdk/iluabase.h"

namespace luaapi {
	namespace detail {
		// hook name:hook id, function reference
		extern std::unordered_map<std::string, std::unordered_map<std::string, int>> hooks;
	}

	void AddHook(const std::string& name, const std::string& id);
	void RemoveHook(const std::string& name, const std::string& id);
	void RemoveAllHooks();
	void CallHook(const std::string& name, int numArgs, std::function<void(ILuaBase*)> pushArgsFunc);

	LUA_FUNCTION_EXTERN(LAddHook);
	LUA_FUNCTION_EXTERN(LRemoveHook);

	LUA_FUNCTION_EXTERN(StartPrediction);
	LUA_FUNCTION_EXTERN(FinishPrediction);

	LUA_FUNCTION_EXTERN(StartSimulation);
	LUA_FUNCTION_EXTERN(SimulateTick);
	LUA_FUNCTION_EXTERN(FinishSimulation);

	LUA_FUNCTION_EXTERN(PredictSpread);
	LUA_FUNCTION_EXTERN(GetServerTime);
	LUA_FUNCTION_EXTERN(SetCommandTick);
	LUA_FUNCTION_EXTERN(SetTyping);
	LUA_FUNCTION_EXTERN(GetSimulationTime);
	LUA_FUNCTION_EXTERN(GetLatency);
	LUA_FUNCTION_EXTERN(GetTargetLBY);
	LUA_FUNCTION_EXTERN(GetCurrentLBY);

	// INetChannel
	LUA_FUNCTION_EXTERN_GETSET(OutSequenceNr);
	LUA_FUNCTION_EXTERN_GETSET(InSequenceNr);
	LUA_FUNCTION_EXTERN_GETSET(OutSequenceNrAck);
	LUA_FUNCTION_EXTERN_GETSET(NetChokedPackets);

	// CClientState
	LUA_FUNCTION_EXTERN_GETSET(LastCommandAck);
	LUA_FUNCTION_EXTERN_GETSET(LastOutgoingCommand);
	LUA_FUNCTION_EXTERN_GETSET(ChokedCommands);

	LUA_FUNCTION_EXTERN(NetSetConVar);

	LUA_FUNCTION_EXTERN(SetShouldChoke);
	LUA_FUNCTION_EXTERN(SetForceChoke);
	

	void PushLuaApi();
}
