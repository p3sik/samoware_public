
#pragma once

#include <Windows.h>
#include <Psapi.h>
#include <cstdint>
#include <vector>

#define INRANGE(x, a, b) (x >= a && x <= b) 
#define GETBITS(x) (INRANGE((x & (~0x20)), 'A', 'F') ? ((x & (~0x20)) - 'A' + 0xA) : (INRANGE(x, '0', '9') ? x - '0' : 0))
#define GETBYTE(x) (GETBITS(x[0]) << 4 | GETBITS(x[1]))

#define VPROXY(methodName, methodIndex, retType, args, ...)					\
	static constexpr std::uintptr_t vIndex_ ## methodName = methodIndex;	\
	retType methodName args noexcept { return cfw::vmt::call<retType>((void*)this, methodIndex, ## __VA_ARGS__); }

namespace cfw {
	static std::uintptr_t findPattern(const char* moduleName, const char* pattern) noexcept {
		HMODULE module = GetModuleHandleA(moduleName);

		MODULEINFO moduleInfo;
		GetModuleInformation(GetCurrentProcess(), module, &moduleInfo, sizeof(MODULEINFO));

		std::uintptr_t scanAddrStart = reinterpret_cast<std::uintptr_t>(module);
		std::uintptr_t scanAddrEnd = scanAddrStart + moduleInfo.SizeOfImage;

		for (std::uintptr_t scanAddress = scanAddrStart; scanAddress < scanAddrEnd; scanAddress++) {
			bool found = true;

			std::uintptr_t addrOffset = 0;
			for (const char* patternChar = pattern; *patternChar;) {
				if (*patternChar == ' ') {
					patternChar++;
					continue;
				} else if (*patternChar == '?') {
					patternChar += 2;
					addrOffset++;
					continue;
				}

				if (*reinterpret_cast<uint8_t*>(scanAddress + addrOffset) != GETBYTE(patternChar)) {
					found = false;
					break;
				}

				patternChar += 2;
				addrOffset++;
			}

			if (found)
				return scanAddress;
		}

		return NULL;
	}

	static void* getAbsAddr(std::uintptr_t inst, std::uintptr_t instOffset = 3, std::uintptr_t instSize = 7) {
		#ifdef _WIN64
		int offset = *reinterpret_cast<int*>(inst + instOffset);
		std::uintptr_t rip = inst + instSize;

		return reinterpret_cast<void*>(rip + static_cast<std::uintptr_t>(offset));
		#else
		return reinterpret_cast<void*>(inst);
		#endif
	}

	namespace vmt {
		// Get VMT
		static inline void** get(void* obj) {
			return *reinterpret_cast<void***>(obj);
		}

		// Get method from object by index
		template <typename T>
		static inline T get(void* obj, std::uintptr_t index) {
			return reinterpret_cast<T>(get(obj)[index]);
		}

		// Get method from vmt by index
		template <typename T>
		static inline T get(void** vmt, std::uintptr_t index) {
			return reinterpret_cast<T>(vmt[index]);
		}

		// Call method by index
		template <typename Ret_t, typename ...Args>
		static inline Ret_t call(void* obj, std::uintptr_t index, Args ...args) noexcept {
			using Function_t = Ret_t(__fastcall*)(void*, decltype(args)...);
			return get<Function_t>(obj, index)(obj, args...);
		}
	}
}
