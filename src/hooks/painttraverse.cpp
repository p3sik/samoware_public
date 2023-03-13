
#include "samoware/hooks/painttraverse.h"

#include "samoware/cheats/lualoader.h"
#include "samoware/cheats/luaapi.h"

#include "samoware/cheat.h"

namespace hooks {
	void __fastcall PaintTraverseHookFunc(IPanel* self, VPANEL vguiPanel, bool forceRepaint, bool allowForce) {
		PaintTraverseHook::Get().GetOriginal()(self, vguiPanel, forceRepaint, allowForce);

		static VPANEL overlayPopupPanel = 0;
		if (!overlayPopupPanel && !strcmp(self->GetName(vguiPanel), "OverlayPopupPanel"))
			overlayPopupPanel = vguiPanel;

		if (vguiPanel != overlayPopupPanel)
			return;

		typedef void(*MsgFn)(const char*, ...);
		static MsgFn msg = reinterpret_cast<MsgFn>(GetProcAddress(GetModuleHandle("tier0.dll"), "Msg"));
		// msg("SW OverlayPopupPanel\n");

		luaapi::CallHook("OverlayPopupPanel",01, [&](ILuaBase* LUA) { });

		LuaLoader::Get().ProcessQueue();

		self->SetMouseInputEnabled(vguiPanel, Samoware::Get().menu->isOpened && !Samoware::Get().shouldUnload);
		self->SetKeyBoardInputEnabled(vguiPanel, Samoware::Get().menu->isOpened && !Samoware::Get().shouldUnload && ImGui::GetIO().WantCaptureKeyboard);
	}
}
 