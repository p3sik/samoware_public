
#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

namespace ImGuiCustom {
	bool BeginGroupPanel(const char* name, const ImVec2& size) {
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		ImDrawList* drawList = window->DrawList;
		const ImGuiStyle& style = g.Style;

		drawList->ChannelsSplit(2);
		drawList->ChannelsSetCurrent(1);

		ImGui::BeginGroup();

		const ImVec2& cursorPos = ImGui::GetCursorScreenPos();
		const ImVec2 itemSpacing = style.ItemSpacing;
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, {0.f, 0.f});
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {0.f, 0.f});

		float frameHeight = ImGui::GetFrameHeight();
		ImGui::BeginGroup();

		auto effectiveSize = size;
		if (size.x < 0.f)
			effectiveSize.x = ImGui::GetContentRegionAvail().x;
		else
			effectiveSize.x = size.x;

		ImGui::Dummy({effectiveSize.x, frameHeight / 2.f});

		ImGui::Dummy({frameHeight * 0.5f, 0.f});
		ImGui::SameLine();
		ImGui::BeginGroup();
		ImGui::Dummy(ImVec2(frameHeight * 0.5f, 0.0f));

		const ImVec2& textSize = ImGui::CalcTextSize(name, 0, true);

		ImGui::SetCursorPos(ImVec2(
			ImGui::GetCursorPos().x + ((effectiveSize.x - frameHeight) / 2 - textSize.x / 2)
			/*- GetStyle().ItemSpacing.x*/, ImGui::GetCursorPos().y - itemSpacing.y));
		ImGui::TextUnformatted(name, ImGui::FindRenderedTextEnd(name));

		ImGui::SameLine();
		ImGui::Dummy({0.0, frameHeight + itemSpacing.y});
		ImGui::BeginGroup();

		ImGui::PopStyleVar(2);

		window->ContentRegionRect.Max.x -= frameHeight * 0.5f;
		window->Size.x -= frameHeight;

		ImGui::PushItemWidth(effectiveSize.x - frameHeight);

		return true;
	}

	void EndGroupPanel() {
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return;

		ImGuiContext& g = *GImGui;
		ImDrawList* drawList = window->DrawList;
		const ImGuiStyle& style = g.Style;

		ImGui::PopItemWidth();

		const ImVec2& itemSpacing = style.ItemSpacing;

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0f, 0.0f));

		float frameHeight = ImGui::GetFrameHeight();

		ImGui::EndGroup();
		ImGui::EndGroup();

		ImGui::SameLine(0.0f, 0.0f);
		ImGui::Dummy({frameHeight * 0.5f, 0.0f});
		ImGui::Dummy({0.0, frameHeight - frameHeight * 0.5f - itemSpacing.y});

		ImGui::EndGroup();

		const ImVec2& itemMin = ImGui::GetItemRectMin();
		const ImVec2& itemMax = ImGui::GetItemRectMax();

		ImGui::PopStyleVar(2);

		window->ContentRegionRect.Max.x += frameHeight * 0.5f;
		window->Size.x += frameHeight;

		ImGui::Dummy({0.0f, 0.0f});

		ImGui::EndGroup();

		drawList->ChannelsSetCurrent(0);

		ImU32 outerColor = ImGui::GetColorU32(ImGuiCol_ChildBg);
		ImU32 innerColor = ImGui::GetColorU32(ImGui::GetStyleColorVec4(ImGuiCol_ChildBg) + ImVec4(0.05f, 0.05f, 0.05f, 1.f));
		drawList->AddRectFilled(itemMin, itemMax, outerColor, 2.f);
		drawList->AddRectFilled(itemMin + ImVec2(itemSpacing.x * 0.5f, frameHeight + itemSpacing.y), itemMax - ImVec2(itemSpacing.x * 0.5f, itemSpacing.y), innerColor, style.FrameRounding);
		
		drawList->ChannelsMerge();
	}

	bool SliderScalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags) {
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems)
			return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		float itemWidth = ImGui::CalcItemWidth();

		const ImVec2& labelSize = ImGui::CalcTextSize(label, NULL, true);
		const ImRect frameBB(window->DC.CursorPos + ImVec2(0.f, labelSize.y * 1.5f), window->DC.CursorPos + ImVec2(itemWidth, labelSize.y * 1.5f + style.FramePadding.y * 2.5f));
		const ImRect totalBB(window->DC.CursorPos, frameBB.Max + ImVec2(labelSize.x > 0.0f ? style.ItemInnerSpacing.x : 0.0f, 0.0f));

		bool tempInputAllowed = false; //(flags & ImGuiSliderFlags_NoInput) == 0;
		ImGui::ItemSize(totalBB, style.FramePadding.y);
		if (!ImGui::ItemAdd(totalBB, id, &frameBB, tempInputAllowed ? ImGuiItemAddFlags_Focusable : 0))
			return false;

		// Default format string when passing NULL
		if (format == NULL)
			format = ImGui::DataTypeGetInfo(data_type)->PrintFmt;

		// Tabbing or CTRL-clicking on Slider turns it into an input box
		const bool hovered = ImGui::ItemHoverable(frameBB, id);
		bool tempInputIsActive = tempInputAllowed && ImGui::TempInputIsActive(id);
		if (!tempInputIsActive) {
			const bool focusRequested = tempInputAllowed && (g.LastItemData.StatusFlags & ImGuiItemStatusFlags_Focused) != 0;
			const bool clicked = (hovered && g.IO.MouseClicked[0]);
			if (focusRequested || clicked || g.NavActivateId == id || g.NavInputId == id) {
				ImGui::SetActiveID(id, window);
				ImGui::SetFocusID(id, window);
				ImGui::FocusWindow(window);
				g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
				if (tempInputAllowed && (focusRequested || (clicked && g.IO.KeyCtrl) || g.NavInputId == id))
					tempInputIsActive = true;
			}
		}

		if (tempInputIsActive) {
			// Only clamp CTRL+Click input when ImGuiSliderFlags_AlwaysClamp is set
			const bool is_clamp_input = (flags & ImGuiSliderFlags_AlwaysClamp) != 0;
			return ImGui::TempInputScalar(frameBB, id, label, data_type, p_data, format, is_clamp_input ? p_min : NULL, is_clamp_input ? p_max : NULL);
		}

		// Draw frame
		const ImU32& frameColor = ImGui::GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
		ImGui::RenderNavHighlight(frameBB, id);
		ImGui::RenderFrame(frameBB.Min, frameBB.Max, frameColor, true, g.Style.FrameRounding / 2.f);

		// Slider behavior
		ImRect grabBB;
		const bool valueChanged = ImGui::SliderBehavior(frameBB, id, data_type, p_data, p_min, p_max, format, flags, &grabBB);
		if (valueChanged)
			ImGui::MarkItemEdited(id);

		// Render grab
		float grabWidth = grabBB.Max.x - grabBB.Min.x;
		if (grabWidth > 0.f) {
			float grabHeight = grabBB.Max.y - grabBB.Min.y;

			grabBB.Min.x += grabWidth / 4.f;
			grabBB.Max.x -= grabWidth / 4.f;

			grabBB.Min.y -= grabHeight;
			grabBB.Max.y += grabHeight;

			window->DrawList->AddRectFilled(grabBB.Min, grabBB.Max, ImGui::GetColorU32(g.ActiveId == id ? ImGuiCol_SliderGrabActive : ImGuiCol_SliderGrab), style.GrabRounding);
		}

		// Draw label
		if (labelSize.x > 0.0f)
			ImGui::RenderText(ImVec2(totalBB.Min.x + style.ItemInnerSpacing.x, totalBB.Min.y /* + style.FramePadding.y*/), label);

		// Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
		char valueBuf[64];
		const char* valueBufEnd = valueBuf + ImGui::DataTypeFormatString(valueBuf, IM_ARRAYSIZE(valueBuf), data_type, p_data, format);
		const ImVec2& valueSize = ImGui::CalcTextSize(valueBuf, NULL, true);
		//ØüGui::RenderTextClipped(totalBB.Min, totalBB.Max, valueBuf, valueBufEnd, NULL, ImVec2(alignX, 0.f));

		ImGui::RenderTextClipped(ImVec2(totalBB.Max.x - valueSize.x, totalBB.Min.y), totalBB.Max, valueBuf, valueBufEnd, NULL);

		return valueChanged;
	}

	bool SliderInt(const char* label, int* v, int v_min, int v_max, const char* format = nullptr, ImGuiSliderFlags flags = ImGuiSliderFlags_None) {
		return SliderScalar(label, ImGuiDataType_S32, v, &v_min, &v_max, format, flags);
	}

	bool SliderFloat(const char* label, float* v, float v_min, float v_max, const char* format = nullptr, ImGuiSliderFlags flags = ImGuiSliderFlags_None) {
		return SliderScalar(label, ImGuiDataType_Float, v, &v_min, &v_max, format, flags);
	}

	template <size_t ItemsLength>
	bool Combo(const char* label, int* current_item, const char* const (&items)[ItemsLength]) {
		return ImGui::Combo(label, current_item, items, ItemsLength);
	}
}
