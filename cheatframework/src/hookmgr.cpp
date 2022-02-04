
#include <MinHook.h>

#include "cfw/hookmgr.h"
#include "cfw/logger.h"

#include <algorithm>
#include <mutex>

namespace cfw {
	HookManager::HookManager(token) {
		_hooks.reserve(10);

		MH_STATUS status;
		if ((status = MH_Initialize()) != MH_OK)
			Logger::Get().Log<LogLevel::ERROR>("Minhook initialization failed:", MH_StatusToString(status));
	}

	HookManager::~HookManager() {
		RemoveAllHooks();

		MH_STATUS status;
		if ((status = MH_Uninitialize()) != MH_OK)
			Logger::Get().Log<LogLevel::ERROR>("Minhook uninitialization failed:", MH_StatusToString(status));
	}

	bool HookManager::Hook::Setup() {
		MH_STATUS status;
		if ((status = MH_CreateHook(_targetFunc, _hookFunc, &_originalFunc)) != MH_OK) {
			Logger::Get().Log<LogLevel::ERROR>(_name, " MH_CreateHook error: ", MH_StatusToString(status));
			return false;
		}

		if ((status = MH_EnableHook(_targetFunc)) != MH_OK) {
			Logger::Get().Log<LogLevel::ERROR>(_name, " MH_EnableHook error: ", MH_StatusToString(status));
			return false;
		}

		_ready = true;

		return true;
	}

	bool HookManager::Hook::Remove() {
		if (!_ready)
			return false;

		MH_STATUS status;
		if ((status = MH_DisableHook(_targetFunc)) != MH_OK) {
			Logger::Get().Log<LogLevel::ERROR>(_name, " MH_DisableHook error: ", MH_StatusToString(status));
			return false;
		}

		_ready = false;

		return true;
	}

	bool HookManager::CreateHook(const char* hookName, void* targetFunc, void* hookFunc) {
		Logger::Get().Log("Creating hook ", hookName);

		Hook& hook = _hooks[hookName] = Hook(hookName, targetFunc, hookFunc);
		return hook.Setup();
	}

	bool HookManager::CreateHook(const char* hookName, void* obj, std::uintptr_t index, void* hookFunc) {
		void* targetFunc = vmt::get<void*>(obj, index);
		return CreateHook(hookName, targetFunc, hookFunc);
	}

	HookManager::Hook& HookManager::GetHook(const char* hookName) {
		return _hooks[hookName];
	}

	bool HookManager::HookExists(const char* hookName) const {
		return _hooks.find(hookName) != _hooks.end();
	}

	bool HookManager::RemoveHook(const char* hookName) {
		Logger::Get().Log("Removing hook ", hookName);

		auto iter = _hooks.find(hookName);
		if (iter == _hooks.end()) {
			Logger::Get().Log<LogLevel::ERROR>("Failed to find hook by name!");
			return false;
		}

		iter->second.Remove();
		
		_hooks.erase(hookName);

		return true;
	}

	bool HookManager::RemoveAllHooks() {
		Logger& logger = Logger::Get();

		bool success = true;
		for (auto& hook : _hooks) {
			logger.Log("Removing hook ", hook.first);
			if (!hook.second.Remove()) {
				success = false;
				logger.Log<LogLevel::ERROR>("Failed to remove hook ", hook.first);
			}
		}

		_hooks.clear();

		return success;
	}
}
