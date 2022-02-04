
#pragma once

#include "cfw/singleton.h"

class CUserCmd;
class Config;

class Misc : public cfw::Singleton<Misc> {
public:
	bool shouldChangeName;

public:
	Misc(token);

	void BunnyHop(CUserCmd* cmd);
	void AutoStrafe(CUserCmd* cmd);
	void FastWalk(CUserCmd* cmd);
	void UseSpam(CUserCmd* cmd);
	void FlashlightSpam(CUserCmd* cmd);
	void ArmBreaker(CUserCmd* cmd);

	void Lagger();

	void ChangeName();

private:
	Config* _config;

	bool _wasOnGround;

};
