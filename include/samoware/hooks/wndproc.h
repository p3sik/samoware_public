
#pragma once

#include <Windows.h>

#include "cfw/singleton.h"

namespace hooks {
	extern LRESULT WINAPI WndProcHookFunc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	class WndProcHook : public cfw::Singleton<WndProcHook> {
	private:
		WNDPROC _origFunction;
		HWND _hWnd;

	public:
		WndProcHook(token) : _origFunction(nullptr), _hWnd(NULL) {}

		WNDPROC GetOriginal() const {
			return _origFunction;
		}

		bool Setup(HWND hWnd) {
			_origFunction = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WndProcHookFunc));
			return _origFunction != nullptr;
		}

		bool Remove() {
			SetWindowLongPtr(_hWnd, GWLP_WNDPROC, (LONG_PTR)_origFunction);
			return true;
		}
	};
}