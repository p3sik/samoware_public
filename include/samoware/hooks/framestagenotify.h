
#include <cfw/basefunchook.h>

#include "../sdk/chlclient.h"
#include "../interfaces.h"

class CHLClient;

namespace hooks {
	void __fastcall FrameStageNotifyHookFunc(CHLClient* self, ClientFrameStage_t stage);

	class FrameStageNotifyHook : public cfw::BaseFunctionHook<FrameStageNotifyHookFunc, cfw::HookType::VIRTUAL, CHLClient::vIndex_FrameStageNotify>, public cfw::Singleton<FrameStageNotifyHook> {
		friend class cfw::Singleton<FrameStageNotifyHook>;

	private:
		const char* GetHookName() const { return "FrameStageNotify"; }

		void* GetHookTarget() {
			return cfw::vmt::get(interfaces::client);
		}

	public:
		FrameStageNotifyHook(token) {}
	};
}
