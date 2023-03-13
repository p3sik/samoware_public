
#pragma once

#include "cfw/singleton.h"
#include "cfw/imenum.h"

enum class LaggerType {
	ok
};

class Config : public cfw::Singleton<Config> {
public:
	Config(token) {};

	struct {
		bool setupBonesFix = false;
		bool updateClientsideAnimationFix = false;
		bool disableInterpolation = false;
		bool disableSequenceInterpolation = false;
	} hvh;

	struct {
		bool bunnyHop = false;
		bool autoStrafe = false;
		bool legitBhop = false;
		bool fastWalk = false;

		bool useSpam = false;
		bool flashlightSpam = false;
		bool armBreaker = false;
		bool antiOBS = true;
		char nameChangerName[128] = "default";
		bool lagger = false;
		cfw::EnumHelper<LaggerType> laggerType = LaggerType::ok;
		int laggerForce = 0;
		bool laggerHold = false;
		int laggerHoldTicks = 0;
	} misc;
};
