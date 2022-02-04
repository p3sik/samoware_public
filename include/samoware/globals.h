
#pragma once

#include "cfw/singleton.h"

#include "sdk/angle.h"

class Globals : public cfw::Singleton<Globals> {
public:
	Globals(token) {};

	bool shouldChoke = false;
	bool forceChoke = false;

	bool inGameOverlay = false;

	bool isNewTick = false;

	Angle currentViewAngles;
};
