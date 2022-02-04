
#include "samoware/cheats/luaapi.h"

#include "cfw/logger.h"

#include "samoware/cheats/prediction.h"

#include "samoware/sdk/luajit.h"
#include "samoware/sdk/cusercmd.h"
#include "samoware/sdk/cbaseentity.h"
#include "samoware/sdk/iengineclient.h"
#include "samoware/sdk/inetchannel.h"
#include "samoware/sdk/cbaseplayeranimstate.h"
#include "samoware/sdk/icliententitylist.h"
#include "samoware/sdk/cclientstate.h"
#include "samoware/sdk/vstdlib.h"

#include "samoware/interfaces.h"
#include "samoware/globals.h"
#include "samoware/md5.h"

namespace luaapi {
	namespace detail {
		// hook name:hook id, function reference
		std::unordered_map<std::string, std::unordered_map<std::string, int>> hooks;
	}


	void AddHook(const std::string& name, const std::string& id) {
		ILuaBase* lua = interfaces::clientLua;

		lua->CheckType(-1, Lua::FUNCTION);

		int ref = lua->ReferenceCreate();

		detail::hooks[name][id] = ref;

		cfw::Logger::Get().Log<cfw::LogLevel::DEBUG>("Created lua hook ", name, ":", id);
	}

	void RemoveHook(const std::string& name, const std::string& id) {
		ILuaBase* lua = interfaces::clientLua;

		bool found = false;

		auto& hooks = detail::hooks[name];

		if (hooks.find(id) == hooks.end()) {
			cfw::Logger::Get().Log<cfw::LogLevel::DEBUG>("Failed to remove lua hook ", name, ":", id);
			return;
		}

		int ref = hooks[id];
		lua->ReferenceFree(ref);

		hooks.erase(id);

		cfw::Logger::Get().Log<cfw::LogLevel::DEBUG>("Removed lua hook ", name, ":", id);
	}

	void RemoveAllHooks() {
		for (auto& [hookName, hooks] : detail::hooks) {
			for (auto& [hookId, ref] : hooks)
				RemoveHook(hookName, hookId);
		}
	}

	void CallHook(const std::string& name, int numArgs, std::function<void(ILuaBase*)> pushArgsFunc) {
		ILuaBase* lua = interfaces::clientLua;
		if (!lua || !interfaces::engineClient->IsInGame()) {
			// Leaved the server, clear hooks
			detail::hooks.clear();
			return;
		}

		auto& hooks = detail::hooks[name];
		for (auto it = hooks.begin(); it != hooks.end(); ) {
			const std::string& id = it->first;
			int ref = it->second;

			lua->ReferencePush(ref);

			if (!lua->IsType(-1, Lua::FUNCTION)) {
				// Registry was corrupted/joined to another server
				it = hooks.erase(it);
				continue;
			}

			pushArgsFunc(lua);

			if (lua->PCall(numArgs, 0, 0)) {
				const char* errorMessage = lua->GetString(-1);
				cfw::Logger::Get().Log<cfw::LogLevel::ERROR>("Error while executing hook ", name, ":", id, ": ", errorMessage);

				lua->Pop();
			}

			it++;
		}
	}

	LUA_FUNCTION(LAddHook) {
		LUA->CheckString(1);
		LUA->CheckString(2);
		LUA->CheckType(3, Lua::FUNCTION);

		const char* name = LUA->GetString(1);
		const char* id = LUA->GetString(2);

		AddHook(name, id);

		return 1;
	}

	LUA_FUNCTION(LRemoveHook) {
		LUA->CheckString(1);
		LUA->CheckString(2);

		const char* name = LUA->GetString(1);
		const char* id = LUA->GetString(2);

		RemoveHook(name, id);

		return 1;
	}

	LUA_FUNCTION(StartPrediction) {
		LUA->CheckType(1, Lua::USERCMD);

		EnginePrediction::Get().Start(LUA->GetUserType<CUserCmd>(1, Lua::USERCMD));

		return 1;
	}

	LUA_FUNCTION(FinishPrediction) {
		EnginePrediction::Get().Finish();

		return 1;
	}

	LUA_FUNCTION(StartSimulation) {
		LUA->CheckNumber(1);
		LUA->CheckType(2, Lua::TABLE);

		int plyIdx = LUA->GetNumber(1);
		CBasePlayer* ply = reinterpret_cast<CBasePlayer*>(interfaces::entityList->GetClientEntity(plyIdx));

		auto getNumber = [&](const char* fieldName) -> double {
			LUA->GetField(2, fieldName);
			if (LUA->IsType(-1, Lua::NUMBER))
				return LUA->GetNumber(-1);

			return 0.f;
		};

		auto getAngle = [&](const char* fieldName) -> Angle {
			LUA->GetField(2, fieldName);
			if (LUA->IsType(-1, Lua::ANGLE))
				return LUA->GetAngle(-1);

			return {0.f, 0.f, 0.f};
		};

		CUserCmd simulatedCmd;
		simulatedCmd.Reset();

		simulatedCmd.viewangles = getAngle("viewangles");
		simulatedCmd.forwardmove = getNumber("forwardmove");
		simulatedCmd.sidemove = getNumber("sidemove");
		simulatedCmd.upmove = getNumber("upmove");
		simulatedCmd.buttons = getNumber("buttons");

		simulatedCmd.hasbeenpredicted = true;

		EnginePrediction::Get().StartSimulation(ply, &simulatedCmd);

		return 1;
	}

	LUA_FUNCTION(SimulateTick) {
		LUA->CheckNumber(1);
		LUA->CheckNumber(2);
		LUA->CheckType(3, Lua::TABLE);

		int plyIdx = LUA->GetNumber(1);
		CBasePlayer* ply = reinterpret_cast<CBasePlayer*>(interfaces::entityList->GetClientEntity(plyIdx));

		int numTick = LUA->GetNumber(2);

		auto getNumber = [&](const char* fieldName) -> double {
			LUA->GetField(3, fieldName);
			if (LUA->IsType(-1, Lua::NUMBER))
				return LUA->GetNumber(-1);

			return 0.f;
		};

		auto getAngle = [&](const char* fieldName) -> Angle {
			LUA->GetField(3, fieldName);
			if (LUA->IsType(-1, Lua::ANGLE))
				return LUA->GetAngle(-1);

			return {0.f, 0.f, 0.f};
		};

		CUserCmd simulatedCmd;
		simulatedCmd.Reset();

		simulatedCmd.viewangles = getAngle("viewangles");
		simulatedCmd.forwardmove = getNumber("forwardmove");
		simulatedCmd.sidemove = getNumber("sidemove");
		simulatedCmd.upmove = getNumber("upmove");
		simulatedCmd.buttons = getNumber("buttons");

		simulatedCmd.hasbeenpredicted = true;

		EnginePrediction::Get().SimulateTick(ply, &simulatedCmd, numTick);

		return 1;
	}

	LUA_FUNCTION(FinishSimulation) {
		LUA->CheckNumber(1);

		int plyIdx = LUA->GetNumber(1);
		CBasePlayer* ply = reinterpret_cast<CBasePlayer*>(interfaces::entityList->GetClientEntity(plyIdx));
		EnginePrediction::Get().FinishSimulation(ply);

		return 1;
	}

	LUA_FUNCTION(PredictSpread) {
		LUA->CheckType(1, Lua::USERCMD);
		LUA->CheckType(2, Lua::ANGLE);
		LUA->CheckNumber(3);

		CUserCmd* cmd = LUA->GetUserType<CUserCmd>(1, Lua::USERCMD);
		Angle angle = LUA->GetAngle(2);
		float weaponSpread = LUA->GetNumber(3) * -1.f;

		uint8_t seed;
		{
			Chocobo1::MD5 md5;
			md5.addData(&cmd->command_number, sizeof(cmd->command_number));
			md5.finalize();

			seed = *reinterpret_cast<uint32_t*>(md5.toArray().data() + 6) & 0xFF;
		}

		vstdlib::RandomSeed(seed);

		float spreadY = vstdlib::RandomFloat(-0.5, 0.5) + vstdlib::RandomFloat(-0.5, 0.5);
		float spreadX = vstdlib::RandomFloat(-0.5, 0.5) + vstdlib::RandomFloat(-0.5, 0.5);

		Vector spreadDir = Vector(1.f, weaponSpread * spreadY, -weaponSpread * spreadX);

		LUA->PushVector(spreadDir);

		return 1;
	}

	LUA_FUNCTION(GetServerTime) {
		LUA->CheckType(1, Lua::USERCMD);

		CUserCmd* cmd = LUA->GetUserType<CUserCmd>(1, Lua::USERCMD);

		LUA->PushNumber(EnginePrediction::Get().GetServerTime(cmd));

		return 1;
	}

	LUA_FUNCTION(SetCommandTick) {
		LUA->CheckType(1, Lua::USERCMD);
		LUA->CheckNumber(2);

		CUserCmd* cmd = LUA->GetUserType<CUserCmd>(1, Lua::USERCMD);
		int tick = LUA->GetNumber(2);

		cmd->tick_count = tick;

		return 1;
	}

	LUA_FUNCTION(SetTyping) {
		LUA->CheckType(1, Lua::USERCMD);
		LUA->CheckType(2, Lua::BOOL);

		CUserCmd* cmd = LUA->GetUserType<CUserCmd>(1, Lua::USERCMD);
		bool typing = LUA->GetBool(2);

		cmd->istyping = typing;
		cmd->unk = typing;

		return 1;
	}

	LUA_FUNCTION(GetSimulationTime) {
		LUA->CheckNumber(1);

		int plyIdx = LUA->GetNumber(1);
		CBasePlayer* ply = reinterpret_cast<CBasePlayer*>(interfaces::entityList->GetClientEntity(plyIdx));

		LUA->PushNumber(ply->m_flSimulationTime());

		return 1;
	}

	LUA_FUNCTION(GetLatency) {
		LUA->CheckNumber(1);

		int flow = LUA->GetNumber(1);

		INetChannel* netChan = interfaces::engineClient->GetNetChannel();
		float latency = netChan->GetLatency(flow);

		LUA->PushNumber(latency);

		return 1;
	}

	LUA_FUNCTION(GetTargetLBY) {
		LUA->CheckNumber(1);

		int plyIdx = LUA->GetNumber(1);
		CBasePlayer* ply = reinterpret_cast<CBasePlayer*>(interfaces::entityList->GetClientEntity(plyIdx));

		CBasePlayerAnimState* animState = ply->GetAnimState();

		LUA->PushNumber(animState->m_flGoalFeetYaw);

		return 1;
	}

	LUA_FUNCTION(GetCurrentLBY) {
		LUA->CheckNumber(1);

		int plyIdx = LUA->GetNumber(1);
		CBasePlayer* ply = reinterpret_cast<CBasePlayer*>(interfaces::entityList->GetClientEntity(plyIdx));

		CBasePlayerAnimState* animState = ply->GetAnimState();

		LUA->PushNumber(animState->m_flCurrentFeetYaw);

		return 1;
	}

	// NetChannel
	LUA_FUNCTION_GETSET(OutSequenceNr, Number, interfaces::engineClient->GetNetChannel()->m_nOutSequenceNr);
	LUA_FUNCTION_GETSET(InSequenceNr, Number, interfaces::engineClient->GetNetChannel()->m_nInSequenceNr);
	LUA_FUNCTION_GETSET(OutSequenceNrAck, Number, interfaces::engineClient->GetNetChannel()->m_nOutSequenceNrAck);
	LUA_FUNCTION_GETSET(NetChokedPackets, Number, interfaces::engineClient->GetNetChannel()->m_nChokedPackets);

	// CClientState
	LUA_FUNCTION_GETSET(LastCommandAck, Number, interfaces::clientState->last_command_ack);
	LUA_FUNCTION_GETSET(LastOutgoingCommand, Number, interfaces::clientState->lastoutgoingcommand);
	LUA_FUNCTION_GETSET(ChokedCommands, Number, interfaces::clientState->chokedcommands);

	LUA_FUNCTION(NetSetConVar) {
		LUA->CheckString(1);
		LUA->CheckString(2);

		const char* conVar = LUA->CheckString(1);
		const char* value = LUA->CheckString(2);

		INetChannel* netChan = interfaces::engineClient->GetNetChannel();

		uint8_t msgBuf[1024];
		NetMessageWriteable netMsg(NetMessage::net_SetConVar, msgBuf, sizeof(msgBuf));
		netMsg.write.WriteUInt(static_cast<uint32_t>(NetMessage::net_SetConVar), NET_MESSAGE_BITS);
		netMsg.write.WriteByte(1); // count
		netMsg.write.WriteString(conVar);
		netMsg.write.WriteString(value);

		netChan->SendNetMsg(netMsg, true);

		return 1;
	}

	LUA_FUNCTION(SetShouldChoke) {
		LUA->CheckType(1, Lua::BOOL);
		Globals::Get().shouldChoke = LUA->GetBool(1);
		
		return 1;
	}

	LUA_FUNCTION(SetForceChoke) {
		LUA->CheckType(1, Lua::BOOL);
		Globals::Get().forceChoke = LUA->GetBool(1);

		return 1;
	}

	// TEMP
	LUA_FUNCTION(SENDDATAGRAM) {
		interfaces::engineClient->GetNetChannel()->SendDatagram(nullptr);
		return 1;
	}

	void PushLuaApi() {
		ILuaBase* luaBase = interfaces::clientLua;

		auto pushFunction = [&](const char* name, CFunc func) {
			luaBase->PushCFunction(func);
			luaBase->SetField(-2, name);
		};

		// Env
		luaBase->CreateTable();

		// Env metatable
		luaBase->CreateTable();
		{
			luaBase->PushSpecial(Lua::SPECIAL_GLOB);
			luaBase->SetField(-2, "__index");
		}
		luaBase->SetMetaTable(-2);

		// samoware table
		luaBase->CreateTable();
		{
			pushFunction("AddHook", LAddHook);
			pushFunction("RemoveHook", LRemoveHook);
			
			pushFunction("StartPrediction", StartPrediction);
			pushFunction("FinishPrediction", FinishPrediction);

			pushFunction("StartSimulation", StartSimulation);
			pushFunction("SimulateTick", SimulateTick);
			pushFunction("FinishSimulation", FinishSimulation);

			pushFunction("PredictSpread", PredictSpread);
			pushFunction("GetServerTime", GetServerTime);
			pushFunction("SetCommandTick", SetCommandTick);
			pushFunction("SetTyping", SetTyping);
			pushFunction("GetSimulationTime", GetSimulationTime);
			pushFunction("GetLatency", GetLatency);
			pushFunction("GetTargetLBY", GetTargetLBY);
			pushFunction("GetCurrentLBY", GetCurrentLBY);

			// INetChannel
			pushFunction("SetOutSequenceNr", SetOutSequenceNr);
			pushFunction("SetInSequenceNr", SetInSequenceNr);
			pushFunction("SetOutSequenceNrAck", SetOutSequenceNrAck);
			pushFunction("SetChokedPackets", SetNetChokedPackets);

			pushFunction("GetOutSequenceNr", GetOutSequenceNr);
			pushFunction("GetInSequenceNr", GetInSequenceNr);
			pushFunction("GetOutSequenceNrAck", GetOutSequenceNrAck);
			pushFunction("GetChokedPackets", GetNetChokedPackets);

			// CClientState
			pushFunction("GetLastCommandAck", GetLastCommandAck);
			pushFunction("GetLastOutgoingCommand", GetLastOutgoingCommand);
			pushFunction("GetChokedCommands", GetChokedCommands);

			pushFunction("SetLastCommandAck", SetLastCommandAck);
			pushFunction("SetLastOutgoingCommand", SetLastOutgoingCommand);
			pushFunction("SetChokedCommands", SetChokedCommands);

			pushFunction("NetSetConVar", NetSetConVar);

			pushFunction("SetShouldChoke", SetShouldChoke);
			pushFunction("SetForceChoke", SetForceChoke);

			// TEMP
			pushFunction("SENDDATAGRAM", SENDDATAGRAM);
		}
		luaBase->SetField(-2, "samoware");

		luajit::call_lua_setfenv(luaBase->GetLuaState(), -2);
	}
}
