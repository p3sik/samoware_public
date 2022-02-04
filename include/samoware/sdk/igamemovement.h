
#pragma once

#include "angle.h"
#include "vector.h"

class CBasePlayer;

class IMoveHelper {
public:
};

class CMoveData {
public:
	char pad_0000[4]; //0x0000
	uint32_t m_nPlayerHandle; //0x0004
	int32_t m_nImpulseCommand; //0x0008
	Angle m_vecViewAngles; //0x000C
	Angle m_vecAbsViewAngles; //0x0018
	int32_t m_nButtons; //0x0024
	int32_t m_nOldButtons; //0x0028
	float m_flForwardMove; //0x002C
	float m_flSideMove; //0x0030
	float m_flUpMove; //0x0034
	float m_flMaxSpeed; //0x0038
	float m_flClientMaxSpeed; //0x003C
	Vector m_vecVelocity; //0x0040
	Angle m_vecAngles; //0x004C
	Angle m_vecOldAngles; //0x0058
	float m_outStepHeight; //0x0064
	Vector m_outWishVel; //0x0068
	Vector m_outJumpVel; //0x0074
	Vector m_vecConstraintCenter; //0x0080
	float m_flConstraintRadius; //0x008C
	float m_flConstraintWidth; //0x0090
	float m_flConstraintSpeedFactor; //0x0094
	Vector m_vecAbsOrigin; //0x0098
	char pad_0001[8]; //0x00A0
};

class IGameMovement {
public:
	virtual			~IGameMovement(void) {}

	// Process the current movement command
	virtual void	ProcessMovement(CBasePlayer* pPlayer, CMoveData* pMove) = 0;
	virtual void	StartTrackPredictionErrors(CBasePlayer* pPlayer) = 0;
	virtual void	FinishTrackPredictionErrors(CBasePlayer* pPlayer) = 0;
	virtual void	DiffPrint(char const* fmt, ...) = 0;

	// Allows other parts of the engine to find out the normal and ducked player bbox sizes
	virtual Vector	GetPlayerMins(bool ducked) const = 0;
	virtual Vector	GetPlayerMaxs(bool ducked) const = 0;
	virtual Vector  GetPlayerViewOffset(bool ducked) const = 0;
};
