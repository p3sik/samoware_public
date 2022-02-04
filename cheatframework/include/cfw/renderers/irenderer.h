
#pragma once

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_dx9.h>
#include <imgui/backends/imgui_impl_win32.h>

namespace cfw {
	class IRenderer {
	protected:
		virtual void BeginRender() {
			ImGui_ImplDX9_NewFrame();
			ImGui_ImplWin32_NewFrame();

			ImGui::NewFrame();
		}

		virtual void FinishRender() {
			ImGui::EndFrame();

			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		}

	public:
		virtual ~IRenderer() {
			Destroy();
		}

		virtual void Setup() = 0;
		virtual void Render() = 0;
		virtual void Destroy() {
			ImGui_ImplDX9_Shutdown();
		};
	};
}
