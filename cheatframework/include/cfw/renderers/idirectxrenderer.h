
#pragma once

#include "irenderer.h"

#include <Windows.h>

#ifdef CFW_RENDERER_DX9
#include <d3d9.h>
#elif CFW_RENDERER_DX10
#include <d3d10.h>
#elif CFW_RENDERER_DX11
#include <d3d11.h>
#else
#error Unknown render backend
#endif

namespace cfw {
	class IDirectXRenderable : public IRenderer {
	protected:
		HWND _hWnd;
		IDirect3DDevice9* _device;

		DWORD _srgbOrig;
		DWORD _colorOrig;

		virtual void BeginRender() {
			// Fix colors
			_device->GetRenderState(D3DRS_SRGBWRITEENABLE, &_srgbOrig);
			_device->GetRenderState(D3DRS_COLORWRITEENABLE, &_colorOrig);
			
			_device->SetRenderState(D3DRS_SRGBWRITEENABLE, 0);
			_device->SetRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF);

			IRenderer::BeginRender();
		}

		virtual void FinishRender() {
			IRenderer::FinishRender();

			_device->SetRenderState(D3DRS_SRGBWRITEENABLE, _srgbOrig);
			_device->SetRenderState(D3DRS_COLORWRITEENABLE, _colorOrig);
		}

		virtual void Setup() = 0;

	public:
		virtual void Setup(HWND hWnd, IDirect3DDevice9* device) {
			_hWnd = hWnd;
			_device = device;

			_srgbOrig = 0;
			_colorOrig = 0;

			Setup();
		}
	};
}