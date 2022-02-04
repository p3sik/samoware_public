
#pragma once

#include "cusercmd.h"

#include <cstdint>

#define MULTIPLAYER_BACKUP 90

class CInput {
public:
	CVerifiedUserCmd* GetVerifiedCommands() {
		return *reinterpret_cast<CVerifiedUserCmd**>(reinterpret_cast<std::uintptr_t>(this) + 0xD8);
	}

	CVerifiedUserCmd* GetVerifiedCommand(int seq) {
		CVerifiedUserCmd* verifiedCmd = reinterpret_cast<CVerifiedUserCmd*>(reinterpret_cast<std::uintptr_t>(GetVerifiedCommands()) + 328 * (seq % MULTIPLAYER_BACKUP));
		return verifiedCmd;
	}
};
