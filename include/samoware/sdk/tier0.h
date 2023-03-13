
#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>


namespace tier0 {
	using ThreadHandle_t = HANDLE;

	using ThreadFunc_t = unsigned(__cdecl*)(void* pParam);
	using CreateSimpleThreadFn = ThreadHandle_t(__cdecl*)(ThreadFunc_t pfnThread, void* pParam, unsigned int stackSize);
	using ThreadJoinFn = bool(__cdecl*)(ThreadHandle_t hThread, unsigned int timeout);
	using ReleaseThreadHandleFn = bool(__cdecl*)(ThreadHandle_t hThread);

	extern CreateSimpleThreadFn CreateSimpleThread;
	extern ThreadJoinFn ThreadJoin;
	extern ReleaseThreadHandleFn ReleaseThreadHandle;

	extern bool setup();
}
