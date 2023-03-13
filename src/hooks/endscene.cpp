
#include "samoware/hooks/endscene.h"
#include "samoware/cheat.h"
#include "samoware/config.h"
#include "samoware/globals.h"

#include "cfw/hookmgr.h"

#include <intrin.h>
#pragma intrinsic(_ReturnAddress)

#include "samoware/interfaces.h"

namespace hooks {
	HRESULT __stdcall EndSceneHookFunc(IDirect3DDevice9* self) {
		typedef void(*MsgFn)(const char*, ...);
		static MsgFn msg = reinterpret_cast<MsgFn>(GetProcAddress(GetModuleHandle("tier0.dll"), "Msg"));
		// msg("SW EndScene\n");

		static const void* gameOverlayHook = 0;

		const void* returnAddress = _ReturnAddress();
		if (!gameOverlayHook) {
			MEMORY_BASIC_INFORMATION info;
			VirtualQuery(returnAddress, &info, sizeof(MEMORY_BASIC_INFORMATION));

			char moduleName[MAX_PATH];
			GetModuleFileNameA((HMODULE)info.AllocationBase, moduleName, MAX_PATH);

			if (strstr(moduleName, "gameoverlay"))
				gameOverlayHook = returnAddress;
		}

		bool inGameOverlay = returnAddress == gameOverlayHook;

		Globals::Get().inGameOverlay = inGameOverlay;

		EndSceneHook::Get().renderable->Render();

		HRESULT result = EndSceneHook::Get().GetOriginal()(self);

		if (Samoware::Get().shouldUnload)
			Samoware::Get().Unload();

		return result;
	}

	void EndSceneHook::InitializeDevice() {
		if (device)
			return;

	#ifdef _WIN64
		device = *reinterpret_cast<IDirect3DDevice9**>(cfw::getAbsAddr(cfw::findPattern("shaderapidx9.dll", "3D 7C 01 76 88 74 07 3D 0E 00 07 80 75 34 48 8B 0D") + 14));
	#else
		device = **(reinterpret_cast<IDirect3DDevice9***>(findPattern("shaderapidx9.dll", "A1 ?? ?? ?? ?? 50 8B 08 FF 51 0C") + 1));
	#endif

		assert(device);
	}
}
