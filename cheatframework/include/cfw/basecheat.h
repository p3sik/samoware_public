
#pragma once

#include "hookmgr.h"

namespace cfw {
	class BaseCheat {
	public:
		bool shouldUnload;
		bool unloaded;

	public:
		BaseCheat() : shouldUnload(false), unloaded(false) {}

		virtual void Initialize(HMODULE module) = 0;

		virtual void Unload() {
			unloaded = true;
		};

		virtual ~BaseCheat() {
			shouldUnload = true;
			Unload();
		}

	protected:
		HMODULE _module;
	};
}
