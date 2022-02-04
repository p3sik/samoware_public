
#include "samoware/sdk/vstdlib.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace vstdlib {
	void RandomSeed(uint32_t seed) {
		typedef float(__cdecl* RandomSeedFn)(uint32_t);
		static RandomSeedFn func = reinterpret_cast<RandomSeedFn>(GetProcAddress(GetModuleHandleA("vstdlib.dll"), "RandomSeed"));
		func(seed);
	}


	float RandomFloat(float min, float max) {
		typedef float(__cdecl* RandomFloatFn)(float, float);
		static RandomFloatFn func = reinterpret_cast<RandomFloatFn>(GetProcAddress(GetModuleHandleA("vstdlib.dll"), "RandomFloat"));
		return func(min, max);
	}
}
