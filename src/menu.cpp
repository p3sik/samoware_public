
#define IMGUI_DEFINE_MATH_OPERATORS

#include "samoware/menu.h"
#include "samoware/cheat.h"
#include "samoware/imgui_custom.h"

#include "samoware/cheats/misc.h"
#include "samoware/cheats/lualoader.h"

#include "samoware/sdk/inetchannel.h"
#include "samoware/sdk/iengineclient.h"

#include "samoware/interfaces.h"
#include "samoware/config.h"
#include "samoware/globals.h"

#include "cfw/basefunchook.h"

#include "imgui/imgui_internal.h"

#include <MinHook.h>

static bool drawTabButton(const char* label, const ImVec2& sizeArg, bool active, ImGuiButtonFlags flags = ImGuiButtonFlags_None) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	ImDrawList* drawList = window->DrawList;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	ImVec2 pos = window->DC.CursorPos;
	if ((flags & ImGuiButtonFlags_AlignTextBaseLine) && style.FramePadding.y < window->DC.CurrLineTextBaseOffset) // Try to vertically align buttons that are smaller/have no padding so that text baseline matches (bit hacky, since it shouldn't be a flag)
		pos.y += window->DC.CurrLineTextBaseOffset - style.FramePadding.y;

	ImVec2 size = ImGui::CalcItemSize(sizeArg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

	const ImRect totalBB(pos, pos + size);
	ImGui::ItemSize(size, style.FramePadding.y);
	if (!ImGui::ItemAdd(totalBB, id))
		return false;

	if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat)
		flags |= ImGuiButtonFlags_Repeat;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(totalBB, id, &hovered, &held, flags);

	// Render
	ImRect frameBB({totalBB.Min.x, totalBB.Max.y + 2.f}, totalBB.Max);
	const ImU32& frameColor = ImGui::GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
	drawList->AddRectFilled(frameBB.Min, frameBB.Max, frameColor);

	// TODO: Optimize?
	const ImU32 textColor = ImGui::GetColorU32(active ? ImGuiCol_FrameBg : ImGuiCol_Text);
	ImGui::PushStyleColor(ImGuiCol_Text, textColor);
	ImGui::RenderTextClipped(totalBB.Min + style.FramePadding, totalBB.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &totalBB);
	ImGui::PopStyleColor();

	return pressed;
}

void SamowareMenu::SetupStyle() {
	ImGuiContext& g = *GImGui;
	ImGuiStyle& style = g.Style;

	style.FrameRounding = 3.f;

	constexpr auto getColor = [](float r, float g, float b) -> ImVec4 {
		return ImVec4(r / 255.f, g / 255.f, b / 255.f, 255.f);
	};

	style.Colors[ImGuiCol_FrameBg] = getColor(75, 210, 100);
	style.Colors[ImGuiCol_FrameBgActive] = getColor(75, 210, 100);
	style.Colors[ImGuiCol_FrameBgHovered] = getColor(90, 230, 130);

	style.Colors[ImGuiCol_Button] = getColor(75, 210, 100);
	style.Colors[ImGuiCol_ButtonActive] = getColor(75, 210, 100);
	style.Colors[ImGuiCol_ButtonHovered] = getColor(90, 230, 130);

	style.Colors[ImGuiCol_SliderGrabActive] = getColor(255, 255, 255);
	style.Colors[ImGuiCol_SliderGrab] = getColor(255, 255, 255);

	style.Colors[ImGuiCol_CheckMark] = getColor(0, 60, 0);

	style.Colors[ImGuiCol_ChildBg] = getColor(55, 55, 55);

	ImFontConfig cfg;

	auto& fonts = ImGui::GetIO().Fonts;
	_anonymousPro = fonts->AddFontFromFileTTF("Anonymous_Pro.ttf", 15.f, &cfg);
	_anonymousProBold = fonts->AddFontFromFileTTF("Anonymous_Pro_B.ttf", 18.f, &cfg);
}

void SamowareMenu::DrawTabs() {
	ImVec2 tabButtonSize(125, 24);

	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, {8.f, 8.f});
	ImGui::BeginGroup();
	{
	#define TAB_BUTTON(name, enumName)	\
					if (drawTabButton(name, tabButtonSize, _openedTab == MenuTab::enumName)) { _openedTab = MenuTab::enumName; }	\
					ImGui::SameLine()

		TAB_BUTTON("AIMBOT", AIMBOT);
		TAB_BUTTON("HVH", HVH);
		TAB_BUTTON("ESP", ESP);
		TAB_BUTTON("MISC", MISC);
		TAB_BUTTON("STYLE", STYLE);

	#undef TAB_BUTTON
	}
	ImGui::EndGroup();
	ImGui::PopStyleVar();
}

void SamowareMenu::DrawAimbotTab() {

}

void SamowareMenu::DrawHvHTab() {
	Config& cfg = Config::Get();

	ImGuiCustom::BeginGroupPanel("Resolver", {307.f, 64.f});
	{
		ImGui::Checkbox("SetupBones fix", &cfg.hvh.setupBonesFix);
		ImGui::Checkbox("UpdateClientsideAnimation fix", &cfg.hvh.updateClientsideAnimationFix);
		ImGui::Checkbox("Disable interpolation", &cfg.hvh.disableInterpolation);
		ImGui::Checkbox("Disable sequence interpolation", &cfg.hvh.disableSequenceInterpolation);

		ImGuiCustom::EndGroupPanel();
	}
}

void SamowareMenu::DrawESPTab() {

}

void SamowareMenu::DrawMiscTab() {
	Config& cfg = Config::Get();

	ImGuiCustom::BeginGroupPanel("Movement", {307.f, 64.f});
	{
		ImGui::Checkbox("Bunnyhop", &cfg.misc.bunnyHop);
		ImGui::Checkbox("Auto strafe", &cfg.misc.autoStrafe);
		ImGui::Checkbox("Legit AutoStrafe", &cfg.misc.legitBhop);

		ImGui::Checkbox("Fast walk", &cfg.misc.fastWalk);

		ImGuiCustom::EndGroupPanel();
	}

	ImGui::SameLine();

	ImGuiCustom::BeginGroupPanel("Other", {307.f, 64.f});
	{
		ImGui::Checkbox("Use spam", &cfg.misc.useSpam);
		ImGui::Checkbox("Flashlight spam", &cfg.misc.flashlightSpam);
		ImGui::Checkbox("Arm breaker", &cfg.misc.armBreaker);

		ImGui::Checkbox("AntiOBS", &cfg.misc.antiOBS);

		ImGui::InputText("Name", cfg.misc.nameChangerName, sizeof(cfg.misc.nameChangerName));
		if (interfaces::engineClient->IsInGame()) {
			if (ImGui::Button("Change name"))
				Misc::Get().shouldChangeName = true;
		}

		ImGui::Checkbox("Lagger", &cfg.misc.lagger);
		ImGuiCustom::Combo("Lagger type", cfg.misc.laggerType.Ptr(), {
			"lol"
						   });
		ImGuiCustom::SliderInt("Lagger force", &cfg.misc.laggerForce, 0, 1024, nullptr, ImGuiSliderFlags_Logarithmic);

		static int clickCount = 0;
		if (ImGui::Button("Unload")) {
			clickCount++;

			if (clickCount >= 10)
				Samoware::Get().shouldUnload = true;
		}

		ImGuiCustom::EndGroupPanel();
	}
}

void SamowareMenu::DrawStyleTab() {
	ImGuiContext& g = *GImGui;
	ImGuiStyle& style = g.Style;

	ImGuiCustom::BeginGroupPanel("Colors", {325.f, 64.f});
	{
		ImGuiColorEditFlags flags = ImGuiColorEditFlags_NoInputs;
		ImGui::ColorEdit4("Text", (float*)&style.Colors[ImGuiCol_Text], flags);
		ImGui::ColorEdit4("CheckMark", (float*)&style.Colors[ImGuiCol_CheckMark], flags);
		ImGui::ColorEdit4("FrameBg", (float*)&style.Colors[ImGuiCol_FrameBg], flags);
		ImGui::ColorEdit4("FrameBgActive", (float*)&style.Colors[ImGuiCol_FrameBgActive], flags);
		ImGui::ColorEdit4("FrameBgHovered", (float*)&style.Colors[ImGuiCol_FrameBgHovered], flags);

		ImGuiCustom::EndGroupPanel();
	}

	ImGui::SameLine();

	ImGuiCustom::BeginGroupPanel("Style", {325.f, 64.f});
	{
		ImGuiCustom::SliderFloat("FrameRounding", &style.FrameRounding, 0.f, 16.f);

		ImGuiCustom::EndGroupPanel();
	}
}

void SamowareMenu::DrawTextEditor() {
	if (ImGui::Begin("Lua editor", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar)) {
		ImGui::SetWindowSize(ImVec2(800.f, 600.f), ImGuiCond_FirstUseEver);

		if (ImGui::BeginMenuBar()) {
			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Run")) {
					LuaLoader::Get().AddToQueue(_textEditor.GetText());
				}

				static bool colorizerEnabled = true;
				if (ImGui::Checkbox("Syntax highlight", &colorizerEnabled)) {
					_textEditor.SetColorizerEnable(colorizerEnabled);
				}

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		auto cpos = _textEditor.GetCursorPosition();

		ImGui::Text("%6d/%-6d %6d lines | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, _textEditor.GetTotalLines(),
					_textEditor.CanUndo() ? "*" : " ",
					_textEditor.GetLanguageDefinition().mName.c_str());

		_textEditor.Render("Lua editor");
	}
	ImGui::End();
}

#define COLOR(R, G, B) {R / 255.f, G / 255.f, B / 255.f, 1.f}

std::unordered_map<cfw::LogLevel, ImVec4> logColors = {
	{cfw::LogLevel::DEBUG, COLOR(30, 80, 255)},
	{cfw::LogLevel::INFO, COLOR(105, 210, 255)},
	{cfw::LogLevel::WARNING, COLOR(250, 180, 50)},
	{cfw::LogLevel::ERROR, COLOR(240, 36, 36)}
};

#undef COLOR

void SamowareMenu::DrawLogger() {
	if (ImGui::Begin("Logger", nullptr, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar)) {
		ImGui::SetWindowSize(ImVec2(400.f, 300.f), ImGuiCond_FirstUseEver);

		auto& buffer = cfw::Logger::Get().GetBuffer();
		for (auto& [logLevel, text] : buffer) {
			ImGui::TextUnformatted("[");
			ImGui::SameLine(0, 0);
			ImGui::TextColored(logColors[logLevel], "%s", cfw::Logger::Get().LogLevelToString(logLevel));
			ImGui::SameLine(0, 0);
			ImGui::TextUnformatted("] ");
			ImGui::SameLine(0, 0);
			ImGui::Text("%s", text.c_str());
		}

		if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
			ImGui::SetScrollHereY(1.0f);
	}
	ImGui::End();
}

void SamowareMenu::DrawMenu() {
	constexpr float tabPadding = 8.f;
	ImGui::SetNextWindowSize({(125.f + tabPadding) * 5 + tabPadding, 452.f});

	if (ImGui::Begin("Samoware v2", &isOpened)) {
		ImGui::PushFont(_anonymousProBold);
		DrawTabs();
		ImGui::PopFont();

		ImGui::PushFont(_anonymousPro);
		switch (_openedTab) {
		case MenuTab::AIMBOT:
			DrawAimbotTab();
			break;
		case MenuTab::HVH:
			DrawHvHTab();
			break;
		case MenuTab::ESP:
			DrawESPTab();
			break;
		case MenuTab::MISC:
			DrawMiscTab();
			break;
		case MenuTab::STYLE:
			DrawStyleTab();
			break;
		}
		ImGui::PopFont();
	}
	ImGui::End();

	DrawTextEditor();

	DrawLogger();
}

void SamowareMenu::Render() {
	bool antiOBS = Config::Get().misc.antiOBS;
	bool inGameOverlay = Globals::Get().inGameOverlay;

	bool shouldRenderMenu = isOpened && ((antiOBS && inGameOverlay) || (!antiOBS && !inGameOverlay));

	if (!shouldRenderMenu)
		return;

	BeginRender();
	DrawMenu();
	FinishRender();
}
