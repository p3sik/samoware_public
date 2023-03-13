
#include "samoware/sdk/tier0.h"

#include <bit>

namespace tier0 {
	CreateSimpleThreadFn CreateSimpleThread = nullptr;
	ThreadJoinFn ThreadJoin = nullptr;
	ReleaseThreadHandleFn ReleaseThreadHandle = nullptr;

	bool setup() {
		HMODULE tier0Module = GetModuleHandleA("tier0.dll");
		if (!tier0Module)
			return false;

		CreateSimpleThread = std::bit_cast<CreateSimpleThreadFn>(GetProcAddress(tier0Module, "CreateSimpleThread"));
		if (!CreateSimpleThread)
			return false;

		ThreadJoin = std::bit_cast<ThreadJoinFn>(GetProcAddress(tier0Module, "ThreadJoin"));
		if (!ThreadJoin)
			return false;

		ReleaseThreadHandle = std::bit_cast<ReleaseThreadHandleFn>(GetProcAddress(tier0Module, "ReleaseThreadHandle"));
		if (!ReleaseThreadHandle)
			return false;

		return true;
	}
}
