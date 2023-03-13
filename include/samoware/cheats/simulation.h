
#pragma once

#include "cfw/singleton.h"

#include "../sdk/igamemovement.h"
#include "../sdk/cusercmd.h"

// https://www.unknowncheats.me/forum/team-fortress-2-a/480821-movement-simulation-using-ctfgamemovement-processmovement.html

class CBaseEntity;
class PlayerDataBackup {
public:
	void Store(CBasePlayer* player);
	void Restore(CBasePlayer* player);

private:
	Vector m_vecOrigin;
	Vector m_vecVelocity;
	Vector m_vecBaseVelocity;
	Vector m_vecViewOffset;
	CBaseEntity* m_hGroundEntity;
	int m_fFlags;
	float m_flDucktime;
	float m_flDuckJumpTime;
	bool m_bDucked;
	bool m_bDucking;
	bool m_bInDuckJump;
	float m_flModelScale;
};

class CBasePlayer;
class MovementSimulation : public cfw::Singleton<MovementSimulation> {
public:
	MovementSimulation(token);

	void Start(CBasePlayer* player);
	void SimulateTick();
	void Finish();

	CMoveData& GetMoveData() { return _moveData; }

private:
	void SetupMoveData(CBasePlayer* player);

private:
	CBasePlayer* _player;
	CMoveData _moveData;

	bool _oldInPrediction;
	bool _oldFirstTimePredicted;
	float _oldFrameTime;

	PlayerDataBackup _playerDataBackup;

	CUserCmd _dummyCmd;
};
