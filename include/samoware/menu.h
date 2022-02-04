
#pragma once

#include "cfw/renderers/idirectxrenderer.h"

#include "texteditor.h"

enum class MenuTab {
	AIMBOT,
	HVH,
	ESP,
	MISC,
	STYLE
};

class SamowareMenu : public cfw::IDirectXRenderable {
public:
	bool isOpened;

public:
	void Setup() {
		isOpened = false;

		ImGui::CreateContext();

		ImGui_ImplWin32_Init(_hWnd);
		ImGui_ImplDX9_Init(_device);

		SetupStyle();
		
		_textEditor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
	}

	void Destroy() {
		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();
	}

	virtual void Render();

	TextEditor& GetTextEditor() { return _textEditor; }

public:
	virtual void BeginRender() {
		cfw::IDirectXRenderable::BeginRender();
	}

	virtual void FinishRender() {
		cfw::IDirectXRenderable::FinishRender();
	}

	void SetupStyle();

	void DrawTabs();

	void DrawAimbotTab();
	void DrawHvHTab();
	void DrawESPTab();
	void DrawMiscTab();
	void DrawStyleTab();

	void DrawTextEditor();

	void DrawLogger();

	void DrawMenu();

private:
	MenuTab _openedTab;
	TextEditor _textEditor;

	ImFont* _anonymousPro;
	ImFont* _anonymousProBold;
};
