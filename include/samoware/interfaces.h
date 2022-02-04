
#pragma once

class IEngineClient;
class CHLClient;
class CInput;
class CClientState;
class CGlobalVars;
class IClientEntityList;
class IPanel;
class CPrediction;
class IGameMovement;
class IMoveHelper;
class CLuaShared;
class ILuaBase;

namespace interfaces {
	extern IEngineClient* engineClient;
	extern CHLClient* client;
	extern CInput* input;
	extern CClientState* clientState;
	extern CGlobalVars* globalVars;
	extern IClientEntityList* entityList;
	extern IPanel* panel;
	extern CPrediction* prediction;
	extern IGameMovement* gameMovement;
	extern IMoveHelper* moveHelper;
	extern CLuaShared* luaShared;
	extern ILuaBase* clientLua;

	extern bool runtimeSetup;

	extern bool setup();
	extern bool setupRuntime();
}
