
#include <Windows.h>

#include "samoware/cheat.h"

DWORD WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpReserved) {
	*reinterpret_cast<volatile int*>(0) = 0;

	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		Samoware::Get().Initialize(hinstDLL);
		break;
	case DLL_PROCESS_DETACH:
		//Samoware::Get().Unload();
		break;
	}

	return TRUE;
}
