
#include "samoware/hooks/wndproc.h"

#include "samoware/cheat.h"

#include <imgui/backends/imgui_impl_win32.h>

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace hooks {
	LRESULT WINAPI WndProcHookFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		bool& isMenuOpened = Samoware::Get().menu->isOpened;

		if (uMsg == WM_KEYUP && wParam == VK_INSERT) {
			isMenuOpened = !isMenuOpened;
			return 0;
		}

		if (uMsg == WM_KEYDOWN && wParam == VK_INSERT) {
			return 0;
		}

		if (isMenuOpened) {
			LRESULT result = ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);

			if (uMsg == WM_CHAR)
				return 0;

			if (result)
				return result;
		}

		return CallWindowProc(WndProcHook::Get().GetOriginal(), hWnd, uMsg, wParam, lParam);
	}
}
