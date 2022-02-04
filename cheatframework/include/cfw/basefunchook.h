
#pragma once

#include "hookmgr.h"
#include "logger.h"

#include <assert.h>

namespace cfw {
	enum class HookType {
		STANDARD,
		VIRTUAL
	};

	template <auto hookFunc, HookType hookType, std::uintptr_t funcIndex>
	class BaseFunctionHook {
	protected:
		using Function_t = decltype(hookFunc);

		virtual const char* GetHookName() const = 0;

		virtual void* GetHookTarget() = 0;

		virtual void OnPreSetup() {}
		virtual void OnRemove() {}

	public:
		BaseFunctionHook() {}
		virtual ~BaseFunctionHook() { Remove(); }

		bool Setup() {
			OnPreSetup();

			if constexpr (hookType == HookType::STANDARD) {
				void* targetFunc = GetHookTarget();

				return HookManager::Get().CreateHook(GetHookName(), targetFunc, hookFunc);
			}

			void** vmt = reinterpret_cast<void**>(GetHookTarget());
			Function_t targetFunc = vmt::get<Function_t>(vmt, funcIndex);

			return HookManager::Get().CreateHook(GetHookName(), targetFunc, hookFunc);
		}

		bool Remove() {
			OnRemove();
			return HookManager::Get().RemoveHook(GetHookName());
		}

		HookManager::Hook& GetHook() const {
			return HookManager::Get().GetHook(GetHookName());
		}

		Function_t GetOriginal() const {
			return GetHook().GetOriginal<Function_t>();
		}
	};
}
