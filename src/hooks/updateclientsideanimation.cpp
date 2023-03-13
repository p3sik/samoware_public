
#include "samoware/hooks/updateclientsideanimation.h"

#include "samoware/sdk/cbaseplayeranimstate.h"
#include "samoware/sdk/cglobalvars.h"
#include "samoware/sdk/luajit.h"
#include "samoware/sdk/defines.h"
#include "samoware/sdk/angle.h"

#include "samoware/cheats/prediction.h"

#include "samoware/globals.h"
#include "samoware/config.h"
#include "samoware/interfaces.h"

typedef __int64 _QWORD;
typedef __int32 _DWORD;
typedef __int16 _WORD;
typedef __int8 _BYTE;

namespace hooks {
	void __fastcall UpdateClientsideAnimationHookFunc(CBasePlayer* self) {
		CBasePlayer* localPlayer = reinterpret_cast<CBasePlayer*>(interfaces::entityList->GetClientEntity(interfaces::engineClient->GetLocalPlayer()));
		if (!Config::Get().hvh.updateClientsideAnimationFix)
			return UpdateClientsideAnimationHook::Get().GetOriginal()(self);

		// Update animations at new ticks
		if (!Globals::Get().isNewTick)
			return;

		// Disable interpolation through varmaps just in case
		VarMapping_t& map = self->GetVarMapping();
		for (int i = 0; i < map.m_nInterpolatedEntries; i++) {
			VarMapEntry_t& entry = map.m_Entries[i];
			entry.m_bNeedsToInterpolate = false;
		}

		float simulationTime = self == localPlayer ? EnginePrediction::Get().GetServerTime() : self->m_flSimulationTime();
		
		float oldCurtime = interfaces::globalVars->curtime;
		float oldFrametime = interfaces::globalVars->frametime;
		interfaces::globalVars->curtime = simulationTime + 0.5f;
		interfaces::globalVars->frametime = interfaces::globalVars->interval_per_tick;

		self->m_iEFlags() |= static_cast<int>(EFlags::DIRTY_ABSVELOCITY);
		self->m_fEffects() |= static_cast<int>(EEffects::NOINTERP);

		UpdateClientsideAnimationHook::Get().GetOriginal()(self);

		self->m_fEffects() &= ~static_cast<int>(EEffects::NOINTERP);
		self->m_iEFlags() &= ~static_cast<int>(EFlags::DIRTY_ABSVELOCITY);

		interfaces::globalVars->curtime = oldCurtime;
		interfaces::globalVars->frametime = oldFrametime;
	}
}
