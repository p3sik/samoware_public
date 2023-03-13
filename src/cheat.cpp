
#include "samoware/cheat.h"

#include "samoware/netvars.h"
#include "samoware/interfaces.h"

#include "samoware/sdk/luajit.h"
#include "samoware/sdk/tier0.h"

#include "samoware/hooks/endscene.h"
#include "samoware/hooks/wndproc.h"
#include "samoware/hooks/createmove.h"
#include "samoware/hooks/clientmode_createmove.h"
#include "samoware/hooks/setupbones.h"
#include "samoware/hooks/painttraverse.h"
#include "samoware/hooks/framestagenotify.h"
#include "samoware/hooks/isplayingtimedemo.h"
#include "samoware/hooks/checkforsequencechange.h"
#include "samoware/hooks/runcommand.h"
#include "samoware/hooks/packetstart.h"
#include "samoware/hooks/impacteffect.h"
#include "samoware/hooks/clmove.h"

#include "samoware/hooks/luastringdump.h"

void Samoware::Initialize(HMODULE module) {
	_module = module;

	cfw::Logger::Get().SetLogLevel(cfw::LogLevel::DEBUG);

	interfaces::setup();
	luajit::setup();
	tier0::setup();
	netvars::init();

	menu = new SamowareMenu();

	HWND hWnd = FindWindowA("Valve001", nullptr);
	hooks::EndSceneHook::Get().SetupCustom(hWnd, menu);
	hooks::WndProcHook::Get().Setup(hWnd);

	hooks::FrameStageNotifyHook::Get().Setup();
	hooks::CreateMoveHook::Get().Setup();
	hooks::ClientModeCreateMoveHook::Get().Setup();
	hooks::PaintTraverseHook::Get().Setup();
	hooks::SetupBonesHook::Get().Setup();
	hooks::IsPlayingTimeDemoHook::Get().Setup();
	hooks::CheckForSequenceChangeHook::Get().Setup();
	// hooks::RunCommandHook::Get().Setup();
	// hooks::PacketStartHook::Get().Setup();
	hooks::impact::hook();
	hooks::CL_MoveHook::Get().Setup();
	
	hooks::StringDumpHook::Get().Setup();
}

void Samoware::Unload() {
	if (unloaded)
		return;

	cfw::HookManager::Get().RemoveAllHooks();
	hooks::impact::unHook();
	hooks::WndProcHook::Get().Remove();
}
