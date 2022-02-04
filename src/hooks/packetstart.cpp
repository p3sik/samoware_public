
#pragma once

#include "samoware/hooks/packetstart.h"

#include "samoware/sdk/iengineclient.h"

namespace hooks {
	std::deque<OutCommand> cmds;

	void __fastcall PacketStartFuncHook(CClientState* self, int incoming_sequence, int outgoing_acknowledged) {
		if (!interfaces::engineClient->IsInGame())
			return PacketStartHook::Get().GetOriginal()(self, incoming_sequence, outgoing_acknowledged);

        for (auto it = cmds.rbegin(); it != cmds.rend(); ++it) {
            if (!it->is_outgoing)
                continue;

            if (it->command_number == outgoing_acknowledged || outgoing_acknowledged > it->command_number && (!it->is_used || it->previous_command_number == outgoing_acknowledged)) {
                it->previous_command_number = outgoing_acknowledged;
                it->is_used = true;
                PacketStartHook::Get().GetOriginal()(self, incoming_sequence, outgoing_acknowledged);
                break;
            }
        }

        auto result = false;

        for (auto it = cmds.begin(); it != cmds.end();) {
            if (outgoing_acknowledged == it->command_number || outgoing_acknowledged == it->previous_command_number)
                result = true;

            if (outgoing_acknowledged > it->command_number && outgoing_acknowledged > it->previous_command_number)
                it = cmds.erase(it);
            else
                ++it;
        }

        if (!result)
            PacketStartHook::Get().GetOriginal()(self, incoming_sequence, outgoing_acknowledged);
	}
}
