
#pragma once

#include "cfw/basefunchook.h"

#include "../sdk/cclientstate.h"
#include "../interfaces.h"

#include <deque>

namespace hooks {
	struct OutCommand {
		bool is_outgoing;
		bool is_used;
		int command_number;
		int previous_command_number;
	};

	extern std::deque<OutCommand> cmds;

	void __fastcall PacketStartFuncHook(CClientState* self, int incoming_sequence, int outgoing_acknowledged);

	class PacketStartHook : public cfw::BaseFunctionHook<PacketStartFuncHook, cfw::HookType::STANDARD, 0>, public cfw::Singleton<PacketStartHook> {
	private:
		const char* GetHookName() const { return "PacketStart"; }

		void* GetHookTarget() {
			return reinterpret_cast<void*>(cfw::findPattern("engine.dll", "48 8B 01 48 83 C4 20 5B 48 FF A0 ?? ?? ?? ?? 48 83 C4 20 5B C3 CC CC CC CC CC CC CC 89 91 ?? ?? ?? ??") + 28);
		}

	public:
		PacketStartHook(token) {}
	};
}
