
#pragma once

#include <cfw/util.h>

#include "samoware/sdk/angle.h"

class INetChannelInfo;
class INetChannel;

class IEngineClient {
public:
	VPROXY(GetLocalPlayer, 12, int, (void));
	VPROXY(SetViewAngles, 20, void, (Angle viewAngles), viewAngles);
	VPROXY(IsInGame, 26, bool, (void));
	VPROXY(GetNetChannelInfo, 72, INetChannelInfo*, (void));
	VPROXY(IsPlayingTimeDemo, 78, bool, (void));

	INetChannel* GetNetChannel() {
		return reinterpret_cast<INetChannel*>(GetNetChannelInfo());
	}
};
