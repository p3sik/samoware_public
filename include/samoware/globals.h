
#pragma once

#include "cfw/singleton.h"

#include "sdk/angle.h"

class Globals : public cfw::Singleton<Globals> {
public:
	Globals(token) {};

	bool shouldChoke = false;
	bool forceChoke = false;

	bool manipulateInterp = false;
	float targetInterp = 0.f;
	
	bool shiftingTickbase = false;
	int tickbaseShift = 0;
	int ticksAllowed = 0;
	int tickbaseShiftRecharge = 0;

	bool inGameOverlay = false;

	bool isNewTick = false;

	Angle currentViewAngles;
};
