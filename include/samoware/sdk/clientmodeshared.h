
#pragma once

#include <cfw/util.h>

class CUserCmd;

class ClientModeShared {
public:
	VPROXY(CreateMove, 21, bool, (float flInputSampleTime, CUserCmd* cmd), flInputSampleTime, cmd);
};
