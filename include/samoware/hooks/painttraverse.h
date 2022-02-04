
#pragma once

#include "cfw/basefunchook.h"

#include "samoware/sdk/ipanel.h"

#include "samoware/interfaces.h"

class IPanel;

namespace hooks {
	void __fastcall PaintTraverseHookFunc(IPanel* self, VPANEL vguiPanel, bool forceRepaint, bool allowForce);

	class PaintTraverseHook : public cfw::BaseFunctionHook<PaintTraverseHookFunc, cfw::HookType::VIRTUAL, IPanel::vIndex_PaintTraverseFn>, public cfw::Singleton<PaintTraverseHook> {
	private:
		const char* GetHookName() const { return "PaintTraverse"; }

		void* GetHookTarget() {
			return cfw::vmt::get(interfaces::panel);
		}

	public:
		PaintTraverseHook(token) {}
	};
}
