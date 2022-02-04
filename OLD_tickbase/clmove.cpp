
#include <hooks/clmove.h>

#include <MinHook.h>
#include <patternscan.h>

extern "C" {
	void __stdcall CL_MoveHook(float, bool);
}

namespace hooks {
	namespace cl_move {
		CL_MoveFn CL_MoveOrig = nullptr;

		static CL_MoveFn CL_Move;
		bool hook() {
			CL_Move = reinterpret_cast<CL_MoveFn>(findPattern("engine.dll", "40 55 53 48 8D AC 24 38 F0 FF FF B8 C8 10 00 00 E8 ?? ?? ?? ?? 48 2B E0"));

			if (MH_CreateHook(CL_Move, &CL_MoveHook, reinterpret_cast<LPVOID*>(&CL_MoveOrig)) != MH_OK)
				return false;

			if (MH_EnableHook(CL_Move) != MH_OK)
				return false;

			return true;
		}

		bool unhook() {
			if (MH_DisableHook(CL_Move) != MH_OK)
				return false;

			return true;
		}
	}
}
