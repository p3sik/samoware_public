
#pragma once

#include "igamemovement.h"

class CBaseEntity;
class CUserCmd;

class CPrediction {
public:
	virtual			~CPrediction(void) = 0;
	virtual void	Init(void) = 0;
	virtual void	Shutdown(void) = 0;
	virtual void	Update(int startframe, bool validframe, int incoming_acknowledged, int outgoing_command) = 0;
	virtual void	PreEntityPacketReceived(int commands_acknowledged, int current_world_update_packet) = 0;
	virtual void	PostEntityPacketReceived(void) = 0;
	virtual void	PostNetworkDataReceived(int commands_acknowledged) = 0;
	virtual void	OnReceivedUncompressedPacket(void) = 0;
	virtual void	GetViewOrigin(Vector& org) = 0;
	virtual void	SetViewOrigin(Vector& org) = 0;
	virtual void	GetViewAngles(Angle& ang) = 0;
	virtual void	SetViewAngles(Angle& ang) = 0;
	virtual void	GetLocalViewAngles(Angle& ang) = 0;
	virtual void	SetLocalViewAngles(Angle& ang) = 0;
	virtual bool	InPrediction(void) = 0;
	virtual bool	IsFirstTimePredicted(void) = 0;
	virtual int		GetIncomingPacketNumber(void) = 0;
	virtual void	RunCommand_(CBaseEntity* player, CUserCmd* ucmd, IMoveHelper* moveHelper) = 0;
	virtual void	SetupMove(CBaseEntity* player, CUserCmd* ucmd, IMoveHelper* pHelper, CMoveData* move) = 0;
	virtual void	FinishMove(CBaseEntity* player, CUserCmd* ucmd, CMoveData* move) = 0;
	virtual void	SetIdealPitch(CBaseEntity* player, const Vector& origin, const Angle& angles, const Vector& viewheight) = 0;
	virtual void	_Update(bool received_new_world_update, bool validframe, int incoming_acknowledged, int outgoing_command) = 0;

	// For hooking
	VPROXY(RunCommand, 17, void, (CBaseEntity* player, CUserCmd* ucmd, IMoveHelper* moveHelper), player, ucmd, moveHelper);
};
