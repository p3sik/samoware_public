
#include "samoware/cheats/simulation.h"

#include "samoware/sdk/cbaseentity.h"
#include "samoware/sdk/cprediction.h"
#include "samoware/sdk/cglobalvars.h"
#include "samoware/interfaces.h"

void PlayerDataBackup::Store(CBasePlayer* player) {
	m_vecOrigin = player->GetAbsOrigin();
	m_vecVelocity = player->GetVelocity();
	m_vecBaseVelocity = player->GetBaseVelocity();
	m_vecViewOffset = player->GetViewOffset();
	m_hGroundEntity = player->m_hGroundEntity();
	m_fFlags = player->m_fFlags();
	m_flDucktime = player->m_flDucktime();
	m_flDuckJumpTime = player->m_flDuckJumpTime();
	m_bDucked = player->m_bDucked();
	m_bDucking = player->m_bDucking();
	m_bInDuckJump = player->m_bInDuckJump();
	m_flModelScale = player->m_flModelScale();
}

void PlayerDataBackup::Restore(CBasePlayer* player) {
	player->GetAbsOrigin() = m_vecOrigin;
	player->GetVelocity() = m_vecVelocity;
	player->GetBaseVelocity() = m_vecBaseVelocity;
	player->GetViewOffset() = m_vecViewOffset;
	player->m_hGroundEntity() = m_hGroundEntity;
	player->m_fFlags() = m_fFlags;
	player->m_flDucktime() = m_flDucktime;
	player->m_flDuckJumpTime() = m_flDuckJumpTime;
	player->m_bDucked() = m_bDucked;
	player->m_bDucking() = m_bDucking;
	player->m_bInDuckJump() = m_bInDuckJump;
	player->m_flModelScale() = m_flModelScale;
}


MovementSimulation::MovementSimulation(token) : _player(nullptr), _moveData {0} {

}

void MovementSimulation::Start(CBasePlayer* player) {
	_player = player;

	player->GetCurrentCommand() = &_dummyCmd;

	_playerDataBackup.Store(player);

	_oldInPrediction = interfaces::prediction->InPrediction();
	_oldFirstTimePredicted = interfaces::prediction->IsFirstTimePredicted();
	_oldFrameTime = interfaces::globalVars->frametime;

	// the hacks that make it work
	{
		if (player->HasFlag(EntityFlags::DUCKING)) {
			// breaks origin's z if FL_DUCKING is not removed
			player->m_fFlags() &= ~static_cast<int>(EntityFlags::DUCKING); 
			// (mins/maxs will be fine when ducking as long as m_bDucked is true)
			player->m_bDucked() = true;
			player->m_flDucktime() = 0.0f;
			player->m_flDuckJumpTime() = 0.0f;
			player->m_bDucking() = false;
			player->m_bInDuckJump() = false;
		}

		player->m_hGroundEntity() = NULL;

		// player->m_flModelScale() -= 0.03125f; //fixes issues with corners

		if (player->IsOnGround())
			player->GetAbsOrigin().z += 0.03125f; //to prevent getting stuck in the ground

		// for some reason if xy vel is zero it doesn't predict
		// if (fabsf(pPlayer->m_vecVelocity().x) < 0.01f)
		//	pPlayer->m_vecVelocity().x = 0.015f;

		// if (fabsf(pPlayer->m_vecVelocity().y) < 0.01f)
		//	pPlayer->m_vecVelocity().y = 0.015f;
	}

	// Setup move data
	SetupMoveData(player);
}

void MovementSimulation::SimulateTick() {
	if (!_player)
		return;

	interfaces::prediction->GetInPrediction() = true;
	interfaces::prediction->GetIsFirstTimePredicted() = false;
	interfaces::globalVars->frametime = interfaces::globalVars->interval_per_tick;

	interfaces::gameMovement->ProcessMovement(_player, &_moveData);
}

void MovementSimulation::Finish() {
	if (!_player)
		return;

	_player->GetCurrentCommand() = nullptr;

	_playerDataBackup.Restore(_player);

	interfaces::prediction->GetInPrediction() = _oldInPrediction;
	interfaces::prediction->GetIsFirstTimePredicted() = _oldFirstTimePredicted;
	interfaces::globalVars->frametime = _oldFrameTime;

	_player = nullptr;

	memset(&_moveData, 0, sizeof(CMoveData));
	memset(&_playerDataBackup, 0, sizeof(PlayerDataBackup));
}

void MovementSimulation::SetupMoveData(CBasePlayer* player) {
	_moveData.m_bFirstRunOfFunctions = false;
	_moveData.m_bGameCodeMovedPlayer = false;
	_moveData.m_nPlayerHandle = player->GetClientUnknown()->GetRefEHandle();
	_moveData.m_vecVelocity = player->GetVelocity();
	_moveData.m_vecAbsOrigin = player->GetAbsOrigin();
	_moveData.m_vecViewAngles = {player->EyePitch(), player->EyeYaw(), 0.0f};

	Vector forward = _moveData.m_vecViewAngles.Forward();
	Vector right = _moveData.m_vecViewAngles.Right();
	_moveData.m_flForwardMove = (_moveData.m_vecVelocity.y - right.y / right.x * _moveData.m_vecVelocity.x) / (forward.y - right.y / right.x * forward.x);
	_moveData.m_flSideMove = (_moveData.m_vecVelocity.x - forward.x * _moveData.m_flForwardMove) / right.x;
}
