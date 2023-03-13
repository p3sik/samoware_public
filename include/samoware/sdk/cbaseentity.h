
#pragma once

#include "cfw/util.h"

#include "defines.h"
#include "varmap.h"
#include "icliententitylist.h"

#include "../util.h"

class CBasePlayerAnimState;
class CUserCmd;
class matrix3x4_t;
class Vector;
class Angle;

class IHandleEntity {
public:
	virtual ~IHandleEntity() {}
	virtual void SetRefEHandle(const CBaseHandle& handle) = 0;
	virtual const CBaseHandle& GetRefEHandle() const = 0;
};

class IClientUnknown : public IHandleEntity {
public:

};

class CBaseEntity {
public:
	IClientUnknown* GetClientUnknown() { return reinterpret_cast<IClientUnknown*>(this); }

	VPROXY(ShouldInterpolate, 146, bool, (void));

	//NETVAR_(Vector, DT_BaseEntity, m_vecAbsVelocity[0], GetAbsVelocity);
	NETVAR_(Vector, DT_BaseEntity, m_vecVelocity[0], GetVelocity);
	NETVAR_(Vector, DT_BaseEntity, m_vecOrigin, GetAbsOrigin);
	OFFSETVAR(char, m_MoveType, 0x1F4);

	OFFSETVAR(VarMapping_t, GetVarMapping, 40);

	OFFSETVAR(int, m_iEFlags, 0x1F0);
	NETVAR(int, DT_BaseEntity, m_fEffects);
};

class CBaseWeapon : public CBaseEntity {
public:
	VPROXY(GetBulletSpread, 327, const Vector&, (void));

	NETVAR(int, DT_BaseCombatWeapon, m_iClip1);
	NETVAR(int, DT_BaseCombatWeapon, m_iClip2);
	NETVAR(float, DT_BaseCombatWeapon, m_flNextPrimaryAttack);
	NETVAR(float, DT_BaseCombatCharacter, m_flNextAttack);
};

class CBaseAnimating : public CBaseEntity {
public:
	VPROXY(SetupBones, 16, bool, (matrix3x4_t* pBoneToWorldOut, int nMaxBones, int boneMask), pBoneToWorldOut, nMaxBones, boneMask);

	OFFSETVAR(CBasePlayerAnimState*, GetAnimState, 0x3610);
	NETVAR(bool, DT_BaseAnimating, m_bClientSideAnimation);
	NETVAR(float, DT_BaseAnimating, m_flModelScale);
};

class CBasePlayer : public CBaseAnimating {
public:
	MoveType GetMoveType() { return static_cast<MoveType>(m_MoveType()); }
	WaterLevel GetWaterLevel() { return static_cast<WaterLevel>(m_nWaterLevel()); }

	bool HasFlag(EntityFlags flag) { return m_fFlags() & static_cast<int>(flag); }
	bool IsOnGround() { return HasFlag(EntityFlags::ONGROUND); }
	bool IsInWater() { return GetWaterLevel() != WaterLevel::NotInWater; }
	bool IsInNoclip() { return GetMoveType() == MoveType::NOCLIP; }

	VPROXY(UpdateClientsideAnimation, 236, void, (void));

	// CPrediction__RunCommand 2 line
	OFFSETVAR(CUserCmd*, GetCurrentCommand, 0x2C50);

	NETVAR(char, DT_GMOD_Player, m_nWaterLevel);

	NETVAR_(float, DT_GMOD_Player, m_angEyeAngles[0], EyePitch);
	NETVAR_(float, DT_GMOD_Player, m_angEyeAngles[1], EyeYaw);

	NETVAR_(Vector, DT_BasePlayer, m_vecBaseVelocity, GetBaseVelocity);
	NETVAR_(Vector, DT_BasePlayer, m_vecViewOffset[0], GetViewOffset);

	NETVAR(int, DT_BasePlayer, m_nTickBase);
	NETVAR(float, DT_BasePlayer, m_flSimulationTime);
	NETVAR(int, DT_BasePlayer, m_fFlags);
	NETVAR(int, DT_BasePlayer, m_hActiveWeapon);

	NETVAR(bool, DT_BasePlayer, m_bDucked);
	NETVAR(float, DT_BasePlayer, m_flDucktime);
	NETVAR(float, DT_BasePlayer, m_flDuckJumpTime);
	NETVAR(bool, DT_BasePlayer, m_bDucking);
	NETVAR(bool, DT_BasePlayer, m_bInDuckJump);

	NETVAR(CBaseEntity*, DT_BasePlayer, m_hGroundEntity);

	// IDA strings
	OFFSETVAR(float, GetFallVelocity, 92);
};
