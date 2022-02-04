
#pragma once

#include "util.h"
#include "singleton.h"

#include <unordered_map>
#include <string.h>

namespace cfw {
	class HookManager : public Singleton<HookManager> {
	public:
		class Hook {
		private:
			const char* _name;
			void* _originalFunc;
			void* _targetFunc;
			void* _hookFunc;
			bool _ready;

		public:
			Hook() : _name(nullptr), _originalFunc(nullptr), _targetFunc(nullptr), _hookFunc(nullptr), _ready(false) {}
			Hook(const char* hookName, void* hookTarget, void* hookFunc) : _name(hookName), _originalFunc(nullptr), _targetFunc(hookTarget), _hookFunc(hookFunc), _ready(false) {}
			
			template <typename Function_t>
			Function_t GetOriginal() const { return reinterpret_cast<Function_t>(_originalFunc); }

			void* GetHookedAddr() const { return _targetFunc; }

			bool Setup();
			bool Remove();
		};

	private:
		std::unordered_map<const char*, Hook> _hooks;

	public:
		HookManager(token);
		~HookManager();

		bool CreateHook(const char* hookName, void* targetFunc, void* hookFunc);
		bool CreateHook(const char* hookName, void* obj, std::uintptr_t index, void* hookFunc);

		Hook& GetHook(const char* hookName);
		bool HookExists(const char* hookName) const;

		bool RemoveHook(const char* hookName);
		bool RemoveAllHooks();
	};
}
