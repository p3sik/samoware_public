
#pragma once

#include "cfw/basefunchook.h"
#include "cfw/singleton.h"
#include "cfw/util.h"

#include "cfw/renderers/idirectxrenderer.h"

#include <d3d9.h>
#include <assert.h>

namespace hooks {
	typedef HRESULT(__stdcall* EndSceneFn)(IDirect3DDevice9*);
	extern HRESULT __stdcall EndSceneHookFunc(IDirect3DDevice9* self);

	class EndSceneHook : public cfw::BaseFunctionHook<EndSceneHookFunc, cfw::HookType::VIRTUAL, 42>, public cfw::Singleton<EndSceneHook> {
		friend class cfw::Singleton<EndSceneHook>;

	private:
		const char* GetHookName() const { return "EndScene"; }

		void InitializeDevice();
		virtual void* GetHookTarget() {
			return cfw::vmt::get(device);
		}

	public:
		IDirect3DDevice9* device;
		cfw::IDirectXRenderable* renderable;

		EndSceneHook(token) : device(nullptr), renderable(nullptr) {
			InitializeDevice();
		}

		~EndSceneHook() {
			delete renderable;
		}

		bool SetupCustom(HWND hWnd, cfw::IDirectXRenderable* renderable_) {
			renderable = renderable_;
			renderable->Setup(hWnd, device);

			return Setup();
		}
	};
}
