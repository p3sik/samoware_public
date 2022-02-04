
#pragma once

#include "sdk/recv.h"

#include <unordered_map>

namespace netvars {
	extern std::unordered_map<std::string, int> netvars;
	extern void init();
}
