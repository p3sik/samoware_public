
#include "samoware/cheats/prediction.h"

#include "cfw/logger.h"

#include "samoware/sdk/cbaseentity.h"
#include "samoware/sdk/cusercmd.h"
#include "samoware/sdk/cprediction.h"
#include "samoware/sdk/cglobalvars.h"
#include "samoware/sdk/cclientstate.h"
#include "samoware/sdk/icliententitylist.h"
#include "samoware/sdk/iengineclient.h"
#include "samoware/interfaces.h"

EnginePrediction::EnginePrediction(token) : moveData {0} {
	_predictionRandomSeed = reinterpret_cast<int*>(cfw::getAbsAddr(cfw::findPattern("client.dll", "48 85 C9 75 0B C7 05 ?? ?? ?? ?? FF FF FF FF C3 8B 41 30") + 5, 2, 10));
	cfw::Logger::Get().Log<cfw::LogLevel::DEBUG>("predictionRandomSeed: 0x", std::hex, _predictionRandomSeed);
}

float EnginePrediction::GetServerTime(CUserCmd* cmd, int tickOffset) const {
	CBasePlayer* localPlayer = reinterpret_cast<CBasePlayer*>(interfaces::entityList->GetClientEntity(interfaces::engineClient->GetLocalPlayer()));

	static CUserCmd* lastCmd = nullptr;

	if (cmd)
		lastCmd = cmd;

	if (!localPlayer) {
		return interfaces::globalVars->curtime;
	}

	int tickBase = localPlayer->m_nTickBase() + tickOffset;

	if (lastCmd && !lastCmd->hasbeenpredicted)
		tickBase++;

	return tickBase * interfaces::globalVars->interval_per_tick;
}

void EnginePrediction::Start(CUserCmd* cmd) {
	CBasePlayer* localPlayer = reinterpret_cast<CBasePlayer*>(interfaces::entityList->GetClientEntity(interfaces::engineClient->GetLocalPlayer()));

	localPlayer->GetCurrentCommand() = cmd;
	*_predictionRandomSeed = cmd->random_seed;

	//unpredictedFlags = localPlayer->m_fFlags();

	_unpredictedCurtime = interfaces::globalVars->curtime;
	_unpredictedFrametime = interfaces::globalVars->frametime;
	//_unpredictedVelocity = localPlayer->GetVelocity();

	interfaces::globalVars->curtime = GetServerTime(cmd);
	interfaces::globalVars->frametime = interfaces::globalVars->interval_per_tick;

	if (interfaces::clientState->m_nDeltaTick > 0) {
		interfaces::prediction->Update(interfaces::clientState->m_nDeltaTick,
									   interfaces::clientState->m_nDeltaTick > 0,
									   interfaces::clientState->last_command_ack,
									   interfaces::clientState->lastoutgoingcommand + interfaces::clientState->chokedcommands);
	}

	interfaces::prediction->SetLocalViewAngles(cmd->viewangles);

	interfaces::gameMovement->StartTrackPredictionErrors(localPlayer);

	memset(&moveData, 0, sizeof(moveData));

	interfaces::prediction->SetupMove(localPlayer, cmd, interfaces::moveHelper, &moveData);
	interfaces::gameMovement->ProcessMovement(localPlayer, &moveData);
	interfaces::prediction->FinishMove(localPlayer, cmd, &moveData);

	interfaces::gameMovement->FinishTrackPredictionErrors(localPlayer);

	if (interfaces::globalVars->frametime > 0.f) {
		localPlayer->m_nTickBase()++;
	}

	predictedFlags = localPlayer->m_fFlags();
}

void EnginePrediction::Finish() {
	CBasePlayer* localPlayer = reinterpret_cast<CBasePlayer*>(interfaces::entityList->GetClientEntity(interfaces::engineClient->GetLocalPlayer()));

	interfaces::globalVars->curtime = _unpredictedCurtime;
	interfaces::globalVars->frametime = _unpredictedFrametime;

	//localPlayer->GetVelocity() = _unpredictedVelocity;

	//localPlayer->m_fFlags() = unpredictedFlags;

	*_predictionRandomSeed = -1;
	localPlayer->GetCurrentCommand() = nullptr;
}
