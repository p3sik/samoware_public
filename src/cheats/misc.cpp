
#include "samoware/cheats/misc.h"

#include "samoware/sdk/cglobalvars.h"
#include "samoware/sdk/cbaseentity.h"
#include "samoware/sdk/cusercmd.h"
#include "samoware/sdk/bitbuf.h"
#include "samoware/sdk/iengineclient.h"
#include "samoware/sdk/inetchannel.h"
#include "samoware/sdk/icliententitylist.h"
#include "samoware/sdk/vector.h"
#include "samoware/sdk/angle.h"

#include "samoware/config.h"
#include "samoware/interfaces.h"

#define _USE_MATH_DEFINES
#include <math.h>

#define RAD2DEG(rad) (rad * 180.f / M_PI)

Misc::Misc(token) {
	_config = &Config::Get();

	shouldChangeName = false;
	_wasOnGround = true;
}

void Misc::BunnyHop(CUserCmd* cmd) {
	static int numHops = 0;
	numHops = 0;

	if (!_config->misc.bunnyHop)
		return;

	if (!cmd->HasButtons(CUserCmd::IN_JUMP))
		return;

	CBasePlayer* localPlayer = reinterpret_cast<CBasePlayer*>(interfaces::entityList->GetClientEntity(interfaces::engineClient->GetLocalPlayer()));

	MoveType moveType = localPlayer->GetMoveType();
	bool canBhop = localPlayer->IsInWater() || moveType == MoveType::LADDER || moveType == MoveType::NOCLIP;

	if (localPlayer->IsOnGround()) {
		numHops++;

		if (numHops > 6 && canBhop) {
			cmd->UnsetButtons(CUserCmd::IN_JUMP);
			numHops = 0;
			return;
		}

		if (_wasOnGround) {
			_wasOnGround = false;
			cmd->UnsetButtons(CUserCmd::IN_JUMP);
			return;
		}

		_wasOnGround = true;
		return;
	}

	_wasOnGround = false;

	if (!canBhop)
		return;

	cmd->UnsetButtons(CUserCmd::IN_JUMP);
}

void Misc::AutoStrafe(CUserCmd* cmd) {
	if (!_config->misc.autoStrafe)
		return;

	if (!cmd->HasButtons(CUserCmd::IN_JUMP))
		return;

	CBasePlayer* localPlayer = reinterpret_cast<CBasePlayer*>(interfaces::entityList->GetClientEntity(interfaces::engineClient->GetLocalPlayer()));

	MoveType moveType = localPlayer->GetMoveType();
	if (localPlayer->IsInWater() || moveType == MoveType::LADDER || moveType == MoveType::NOCLIP)
		return;

	auto getMaxAngleDelta = [&](float speed) -> float {
		float x = (30.f - (10.f * 300.f * interfaces::globalVars->interval_per_tick)) / speed;
		if (x < 1.f && x > -1.f)
			return acosf(x) + 0.175f;

		return 0.f;
	};

	Vector& velocity = localPlayer->GetVelocity();

	float velYaw = RAD2DEG(atan2f(velocity.y, velocity.x));
	float strafeSide = Angle::Normalize180(velYaw - cmd->viewangles.y) > 0.f ? 1.f : -1.f;

	float maxMove = _config->misc.legitBhop ? 10000.f : 5250.f;

	float forwardMove = cmd->forwardmove;
	if (std::fabsf(forwardMove) < 1.f && localPlayer->IsOnGround() && velocity.Length2D() < 500.f)
		forwardMove = maxMove;

	float sideMove = cmd->forwardmove;
	if (std::fabsf(sideMove) < 1.f) {
		sideMove = maxMove * strafeSide;

		cmd->UnsetButtons(CUserCmd::IN_MOVERIGHT | CUserCmd::IN_MOVELEFT);
		cmd->SetButtons((strafeSide > 0.f) ? CUserCmd::IN_MOVERIGHT : CUserCmd::IN_MOVELEFT);
	}

	if (!_config->misc.legitBhop) {
		cmd->viewangles.y += getMaxAngleDelta(velocity.Length2D()) * strafeSide;
	}

	cmd->buttons |= CUserCmd::IN_SPEED;
	cmd->forwardmove = forwardMove;
	cmd->sidemove = sideMove;
}

void Misc::FastWalk(CUserCmd* cmd) {
	if (!_config->misc.fastWalk)
		return;

	CBasePlayer* localPlayer = reinterpret_cast<CBasePlayer*>(interfaces::entityList->GetClientEntity(interfaces::engineClient->GetLocalPlayer()));

	MoveType moveType = localPlayer->GetMoveType();
	if (localPlayer->IsInWater() || !localPlayer->IsOnGround() || moveType == MoveType::LADDER || moveType == MoveType::NOCLIP)
		return;

	if (cmd->forwardmove > 0.f && !cmd->HasButtons(CUserCmd::IN_MOVERIGHT | CUserCmd::IN_MOVELEFT)) {
		cmd->sidemove = cmd->command_number % 2 ? 5250.f : -5250.f;
	}
}

void Misc::UseSpam(CUserCmd* cmd) {
	if (!_config->misc.useSpam)
		return;

	if (cmd->HasButtons(CUserCmd::IN_USE) && cmd->command_number % 2 == 0)
		cmd->UnsetButtons(CUserCmd::IN_USE);
}

void Misc::FlashlightSpam(CUserCmd* cmd) {
	if (!_config->misc.flashlightSpam)
		return;

	// TODO
	//if (cmd->impulse == CUserCmd::FLASHLIGHT && cmd->command_number % 2 == 0)

}

void Misc::ArmBreaker(CUserCmd* cmd) {
	if (!_config->misc.armBreaker)
		return;

	cmd->istyping = true;
	cmd->unk = true;
}

uint32_t xorshift32() {
	static uint32_t state = 1337;

	uint32_t x = state;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;

	return state = x;
}

void Misc::Lagger() {
	// https://cdn.discordapp.com/emojis/704543785778020493.webp
}

void Misc::ChangeName() {
	if (!shouldChangeName)
		return;

	shouldChangeName = false;

	if (interfaces::engineClient->IsInGame()) {
		// Change with netmessage

		static uint8_t msgBuf[256 + 2 + sizeof(_config->misc.nameChangerName)];

		INetChannel* netChan = interfaces::engineClient->GetNetChannel();

		NetMessageWriteable netMsg(NetMessage::net_SetConVar, msgBuf, sizeof(msgBuf));

		netMsg.write.WriteUInt(static_cast<uint32_t>(NetMessage::net_SetConVar), NET_MESSAGE_BITS);
		netMsg.write.WriteByte(1); // count
		netMsg.write.WriteString("name");
		netMsg.write.WriteString(_config->misc.nameChangerName);

		netChan->SendNetMsg(netMsg, true);
	}
}
