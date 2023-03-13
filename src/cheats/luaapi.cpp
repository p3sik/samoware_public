
#include "samoware/cheats/luaapi.h"

#include "cfw/logger.h"

#include "samoware/cheats/prediction.h"
#include "samoware/cheats/simulation.h"
#include "samoware/cheats/threaded_simulation.h"

#include "samoware/sdk/luajit.h"
#include "samoware/sdk/cusercmd.h"
#include "samoware/sdk/cbaseentity.h"
#include "samoware/sdk/iengineclient.h"
#include "samoware/sdk/inetchannel.h"
#include "samoware/sdk/cbaseplayeranimstate.h"
#include "samoware/sdk/icliententitylist.h"
#include "samoware/sdk/cclientstate.h"
#include "samoware/sdk/vstdlib.h"
#include "samoware/sdk/ienginetrace.h"

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

		int plyIdx = LUA->GetNumber(1);
		CBasePlayer* ply = reinterpret_cast<CBasePlayer*>(interfaces::entityList->GetClientEntity(plyIdx));

		MovementSimulation::Get().Start(ply);

		return 0;
	}

	LUA_FUNCTION(SimulateTick) {
		MovementSimulation::Get().SimulateTick();

		return 0;
	}

	LUA_FUNCTION(GetSimulationData) {
		LUA->CreateTable();

		const auto& moveData = MovementSimulation::Get().GetMoveData();

		LUA->PushAngle(moveData.m_vecViewAngles);
		LUA->SetField(-2, "m_vecViewAngles");

		LUA->PushVector(moveData.m_vecVelocity);
		LUA->SetField(-2, "m_vecVelocity");

		LUA->PushAngle(moveData.m_vecAngles);
		LUA->SetField(-2, "m_vecAngles");

		LUA->PushVector(moveData.m_vecAbsOrigin);
		LUA->SetField(-2, "m_vecAbsOrigin");

		return 1;
	}

	LUA_FUNCTION(FinishSimulation) {
		MovementSimulation::Get().Finish();

		return 0;
	}

	LUA_FUNCTION(EditSimulationData) {
		LUA->CheckType(1, Lua::TABLE);

		auto fieldExists = [&](const char* name) -> bool {
			LUA->GetField(1, name);
			bool isNil = LUA->IsType(-1, Lua::NIL);
			LUA->Pop();
			return !isNil;
		};

		auto& moveData = MovementSimulation::Get().GetMoveData();
		if (fieldExists("m_nButtons")) {
			LUA->GetField(1, "m_nButtons");
			moveData.m_nButtons = LUA->GetNumber();
		}

		if (fieldExists("m_nOldButtons")) {
			LUA->GetField(1, "m_nOldButtons");
			moveData.m_nOldButtons = LUA->GetNumber();
		}

		if (fieldExists("m_flForwardMove")) {
			LUA->GetField(1, "m_flForwardMove");
			moveData.m_flForwardMove = LUA->GetNumber();
		}

		if (fieldExists("m_flSideMove")) {
			LUA->GetField(1, "m_flSideMove");
			moveData.m_flSideMove = LUA->GetNumber();
		}

		if (fieldExists("m_vecViewAngles")) {
			LUA->GetField(1, "m_vecViewAngles");
			moveData.m_vecViewAngles = *LUA->GetUserType<Angle>(-1, Lua::ANGLE);
		}

		return 0;
	}

	template <typename T>
	static bool copyFieldFrom(ILuaBase* lua, const char* name, T& outField) {
		lua->GetField(-1, name);
		if (lua->IsType(-1, Lua::NIL)) {
			lua->Pop();
			return false;
		}

		if constexpr (std::is_same_v<T, bool>) {
			if (!lua->IsType(-1, Lua::BOOL)) {
				char buf[256] {};
				strcpy_s(buf, "Expected bool for field ");
				strcat_s(buf, name);

				lua->Error(buf);
			}

			outField = lua->GetBool(-1);
		} else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float>) {
			if (!lua->IsType(-1, Lua::NUMBER)) {
				char buf[256] {};
				strcpy_s(buf, "Expected number for field ");
				strcat_s(buf, name);

				lua->Error(buf);
			}

			outField = lua->GetNumber(-1);
		} else if constexpr (std::is_same_v<T, Vector>) {
			if (!lua->IsType(-1, Lua::VECTOR)) {
				char buf[256] {};
				strcpy_s(buf, "Expected vector for field ");
				strcat_s(buf, name);

				lua->Error(buf);
			}

			outField = lua->GetVector(-1);
		} else if constexpr (std::is_same_v<T, Angle>) {
			if (!lua->IsType(-1, Lua::ANGLE)) {
				char buf[256] {};
				strcpy_s(buf, "Expected angle for field ");
				strcat_s(buf, name);

				lua->Error(buf);
			}

			outField = lua->GetAngle(-1);
		} else {
			static_assert(false);
		}

		lua->Pop();

		return true;
	}

	template <typename T>
	static bool copyFieldTo(ILuaBase* lua, const char* name, const T& field) {
		if constexpr (std::is_same_v<T, int> || std::is_same_v<T, float>) {
			lua->PushNumber(field);
			lua->SetField(-2, name);
		} else if constexpr (std::is_same_v<T, Vector>) {
			lua->PushVector(field);
			lua->SetField(-2, name);
		} else if constexpr (std::is_same_v<T, Angle>) {
			lua->PushAngle(field);
			lua->SetField(-2, name);
		} else {
			static_assert(false);
		}

		return true;
	}

	// SHITCODE INCOMING
	LUA_FUNCTION(SetThreadedSimulationThreads) {
		LUA->CheckType(1, Lua::NUMBER);

		size_t numThreads = LUA->GetNumber(1);
		ThreadedMovementSimulation::Get().CreateThreads(numThreads);

		return 0;
	}

	LUA_FUNCTION(StartThreadedSimulation) {
		LUA->CheckType(1, Lua::TABLE);

		// #define COPYFIELD(name) copyFieldFrom(LUA, #name, simulationData->##name)
		#define COPYFIELD(name) copyFieldFrom(LUA, #name, simulationData.##name)

		auto& moveSim = ThreadedMovementSimulation::Get();

		int jobIndex = 0;

		LUA->PushNil();
		while (LUA->Next(1)) {
			LUA->CheckType(-1, Lua::TABLE);
			jobIndex++;

			MovementSimulationDataInput simulationData;
			COPYFIELD(timeUntilTimeout);
			COPYFIELD(deltaTime);
			COPYFIELD(idealYawDelta);
			COPYFIELD(maxSpeedLoss);
			COPYFIELD(sandboxAccel);

			COPYFIELD(gravity);
			COPYFIELD(currentGravity);
			COPYFIELD(jumpPower);

			{
				LUA->GetField(-1, "groundEntity");
				if (!LUA->IsType(-1, Lua::ENTITY)) {
					LUA->Error("Expected Entity type for groundEntity");
				}

				// simulationData->groundEntity = LUA->GetUserType<CBaseEntity>(-1, Lua::ENTITY);
				simulationData.groundEntity = LUA->GetUserType<CBaseEntity>(-1, Lua::ENTITY);
				LUA->Pop();
			}

			COPYFIELD(obbMins);
			COPYFIELD(obbMaxs);
			COPYFIELD(m_surfaceFriction);

			COPYFIELD(sv_airaccelerate);

			COPYFIELD(m_vecViewAngles);
			COPYFIELD(m_vecVelocity);
			COPYFIELD(m_vecAbsOrigin);

			COPYFIELD(m_flForwardMove);
			COPYFIELD(m_flSideMove);

			COPYFIELD(m_nButtons);
			COPYFIELD(m_nOldButtons);

			COPYFIELD(m_flMaxSpeed);

			LUA->Pop(); // Pop iterated value

			// moveSim.CreateJob(simulationData);
			moveSim.CreateJob(simulationData, jobIndex);
		}

		// Pop the table
		LUA->Pop(1);

		return 0;
	}

	LUA_FUNCTION(FinishThreadedSimulation) {
		LUA->CheckNumber(1);

		auto& moveSim = ThreadedMovementSimulation::Get();

		// Create main table
		LUA->CreateTable();

		std::stack<MovementSimulationDataOutput> simTicks;
		/*
		int threadIndex = moveSim.ActiveThreads(); // fifo
		while (moveSim.WaitForNext(simTicks)) {
			// Push thread index
			LUA->PushLong(threadIndex--);

			// Create thread table
			LUA->CreateTable();

			int tickIndex = simTicks.size(); // fifo
			while (!simTicks.empty()) {
				// Push tick index
				LUA->PushLong(tickIndex--);

				// Create tick table
				LUA->CreateTable();

				auto& simData = simTicks.top();

				copyFieldTo(LUA, "timeUntilTimeout", simData.timeUntilTimeout);
				copyFieldTo(LUA, "m_vecVelocity", simData.m_vecVelocity);
				copyFieldTo(LUA, "m_vecAbsOrigin", simData.m_vecAbsOrigin);

				simTicks.pop();

				// Insert tick table into the thread table in reversed order
				LUA->SetTable(-3);
			}

			// Insert thread table into the main table
			LUA->SetTable(-3);
		}
		*/

		// Sync
		size_t numJobs = LUA->GetNumber(1);
		moveSim.WaitForAllJobs(numJobs);

		for (auto outputIndex = 0; outputIndex < moveSim.job_outputs.size(); outputIndex++) {
			auto& jobOutput = moveSim.job_outputs[outputIndex];
			auto& simTicks = jobOutput.ticks;

			// Push job index
			LUA->PushLong(jobOutput.index);

			// Create thread table
			LUA->CreateTable();

			for (auto tickIndex = 0; tickIndex < simTicks.size(); tickIndex++) {
				// Push tick index
				LUA->PushLong(tickIndex);

				// Create tick table
				LUA->CreateTable();

				auto& tick = simTicks[tickIndex];

				copyFieldTo(LUA, "timeUntilTimeout", tick.timeUntilTimeout);
				copyFieldTo(LUA, "m_vecVelocity", tick.m_vecVelocity);
				copyFieldTo(LUA, "m_vecAbsOrigin", tick.m_vecAbsOrigin);

				// Insert tick table into the thread table in reversed order
				LUA->SetTable(-3);
			}

			// Insert thread table into the main table
			LUA->SetTable(-3);
		}

		moveSim.ClearJobs();

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

	LUA_FUNCTION(SetCommandNumber) {
		LUA->CheckType(1, Lua::USERCMD);
		LUA->CheckNumber(2);

		CUserCmd* cmd = LUA->GetUserType<CUserCmd>(1, Lua::USERCMD);
		int tick = LUA->GetNumber(2);

		cmd->command_number = tick;

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

	LUA_FUNCTION(SetEntityFlags) {
		LUA->CheckNumber(1);
		LUA->CheckNumber(2);

		int plyIdx = LUA->GetNumber(1);
		CBasePlayer* ply = reinterpret_cast<CBasePlayer*>(interfaces::entityList->GetClientEntity(plyIdx));

		ply->m_fFlags() = static_cast<int>(LUA->GetNumber(2));

		return 0;
	}

	LUA_FUNCTION(CallCLMove) {
		using CL_MoveFn = void(__fastcall*)();
		static CL_MoveFn CL_Move = reinterpret_cast<CL_MoveFn>(cfw::findPattern("engine.dll", "40 55 53 48 8D AC 24 38 F0 FF FF B8 C8 10 00 00 E8 ?? ?? ?? ?? 48 2B E0"));

		CL_Move();

		return 0;
	}

	LUA_FUNCTION(AdjustTickbase) {
		CBasePlayer* localPlayer = reinterpret_cast<CBasePlayer*>(interfaces::entityList->GetClientEntity(interfaces::engineClient->GetLocalPlayer()));
		localPlayer->m_nTickBase()++;

		return 0;
	}

	LUA_FUNCTION(GetShiftingTickbase) {
		auto& globals = Globals::Get();
		LUA->PushBool(globals.shiftingTickbase);

		return 1;
	}

	LUA_FUNCTION(GetTicksAllowed) {
		auto& globals = Globals::Get();
		LUA->PushNumber(globals.ticksAllowed);

		return 1;
	}

	LUA_FUNCTION(SetTickbaseShift) {
		LUA->CheckNumber(1);

		auto& globals = Globals::Get();
		globals.tickbaseShift = static_cast<int>(LUA->GetNumber(1));

		return 0;
	}

	LUA_FUNCTION(GetTickbaseShift) {
		auto& globals = Globals::Get();
		LUA->PushNumber(globals.tickbaseShift);

		return 1;
	}

	LUA_FUNCTION(SetTickbaseRecharge) {
		LUA->CheckNumber(1);

		auto& globals = Globals::Get();
		globals.tickbaseShiftRecharge = static_cast<int>(LUA->GetNumber(1));

		return 0;
	}

	LUA_FUNCTION(GetTickbaseRecharge) {
		auto& globals = Globals::Get();
		LUA->PushNumber(globals.tickbaseShiftRecharge);

		return 1;
	}

	// NetChannel
	LUA_FUNCTION_GETSET(OutSequenceNr, Number, interfaces::engineClient->GetNetChannel()->m_nOutSequenceNr);
	LUA_FUNCTION_GETSET(InSequenceNr, Number, interfaces::engineClient->GetNetChannel()->m_nInSequenceNr);
	LUA_FUNCTION_GETSET(OutSequenceNrAck, Number, interfaces::engineClient->GetNetChannel()->m_nOutSequenceNrAck);
	LUA_FUNCTION_GETSET(NetChokedPackets, Number, interfaces::engineClient->GetNetChannel()->m_nChokedPackets);

	LUA_FUNCTION(SetManipulateInterp) {
		LUA->CheckType(1, Lua::BOOL);
		Globals::Get().manipulateInterp = LUA->GetBool(1);

		return 0;
	}

	LUA_FUNCTION(SetTargetInterp) {
		LUA->CheckNumber(1);
		Globals::Get().targetInterp = LUA->GetNumber(1);

		return 0;
	}

	LUA_FUNCTION(GetManipulateInterp) {
		LUA->PushBool(Globals::Get().manipulateInterp);

		return 1;
	}

	LUA_FUNCTION(GetTargetInterp) {
		LUA->PushNumber(Globals::Get().targetInterp);

		return 1;
	}

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

	LUA_FUNCTION(SENDDATAGRAM) {
		/*int plyIdx = LUA->GetNumber(1);
		CBasePlayer* ply = reinterpret_cast<CBasePlayer*>(interfaces::entityList->GetClientEntity(plyIdx));

		CBasePlayerAnimState* animState = ply->GetAnimState();

		LUA->PushNumber(animState->move_x);
		LUA->PushNumber(animState->move_y);*/

		LUA->CheckType(1, Lua::VECTOR);
		LUA->CheckType(2, Lua::VECTOR);
		LUA->CheckType(3, Lua::VECTOR);
		LUA->CheckType(4, Lua::VECTOR);

		Vector start = LUA->GetVector(1), end = LUA->GetVector(2);
		Vector mins = LUA->GetVector(3), maxs = LUA->GetVector(4);

		Ray_t ray;
		ray.Init(start, end, mins, maxs);

		CTraceFilterWorldOnly filter;

		trace_t trace;
		interfaces::engineTrace->TraceRay(ray, 33636363, &filter, &trace);

		LUA->CreateTable();
		copyFieldTo(LUA, "startpos", trace.startpos);
		copyFieldTo(LUA, "endpos", trace.endpos);
		copyFieldTo(LUA, "fraction", trace.fraction);

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
			pushFunction("GetSimulationData", GetSimulationData);
			pushFunction("FinishSimulation", FinishSimulation);
			pushFunction("EditSimulationData", EditSimulationData);
			
			pushFunction("SetThreadedSimulationThreads", SetThreadedSimulationThreads);
			pushFunction("StartThreadedSimulation", StartThreadedSimulation);
			pushFunction("FinishThreadedSimulation", FinishThreadedSimulation);

			pushFunction("PredictSpread", PredictSpread);
			pushFunction("GetServerTime", GetServerTime);
			pushFunction("SetCommandTick", SetCommandTick);
			pushFunction("SetCommandNumber", SetCommandNumber);
			pushFunction("SetTyping", SetTyping);
			pushFunction("GetSimulationTime", GetSimulationTime);
			pushFunction("GetLatency", GetLatency);
			pushFunction("GetTargetLBY", GetTargetLBY);
			pushFunction("GetCurrentLBY", GetCurrentLBY);
			pushFunction("SetEntityFlags", SetEntityFlags);

			pushFunction("CL_Move", CallCLMove);
			pushFunction("AdjustTickbase", AdjustTickbase);
			pushFunction("GetShiftingTickbase", GetShiftingTickbase);
			pushFunction("GetTicksAllowed", GetTicksAllowed);
			pushFunction("SetTickbaseShift", SetTickbaseShift);
			pushFunction("GetTickbaseShift", SetTickbaseShift);
			pushFunction("SetTickbaseRecharge", SetTickbaseRecharge);
			pushFunction("GetTickbaseRecharge", GetTickbaseRecharge);

			// INetChannel
			pushFunction("SetOutSequenceNr", SetOutSequenceNr);
			pushFunction("SetInSequenceNr", SetInSequenceNr);
			pushFunction("SetOutSequenceNrAck", SetOutSequenceNrAck);
			pushFunction("SetChokedPackets", SetNetChokedPackets);

			pushFunction("GetOutSequenceNr", GetOutSequenceNr);
			pushFunction("GetInSequenceNr", GetInSequenceNr);
			pushFunction("GetOutSequenceNrAck", GetOutSequenceNrAck);
			pushFunction("GetChokedPackets", GetNetChokedPackets);

			pushFunction("SetManipulateInterp", SetManipulateInterp);
			pushFunction("SetTargetInterp", SetTargetInterp);
			pushFunction("GetManipulateInterp", GetManipulateInterp);
			pushFunction("GetTargetInterp", GetTargetInterp);

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
