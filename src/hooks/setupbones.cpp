
#include "samoware/hooks/setupbones.h"

#include "samoware/sdk/defines.h"
#include "samoware/sdk/cglobalvars.h"
#include "samoware/sdk/iengineclient.h"
#include "samoware/sdk/icliententitylist.h"

#include "samoware/cheats/prediction.h"

#include "samoware/interfaces.h"
#include "samoware/config.h"

namespace hooks {
	bool __fastcall SetupBonesHookFunc(CBaseAnimating* self, matrix3x4_t* pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime) {
		CBasePlayer* ply = reinterpret_cast<CBasePlayer*>(self - 8); // Is this even right
		CBasePlayer* localPlayer = reinterpret_cast<CBasePlayer*>(interfaces::entityList->GetClientEntity(interfaces::engineClient->GetLocalPlayer()));

		if (!Config::Get().hvh.setupBonesFix)
			return SetupBonesHook::Get().GetOriginal()(self, pBoneToWorldOut, nMaxBones, boneMask, currentTime);

		float simulationTime = ply == localPlayer ? EnginePrediction::Get().GetServerTime() : ply->m_flSimulationTime();

		float oldCurtime = interfaces::globalVars->curtime;
		float oldFrametime = interfaces::globalVars->frametime;
		interfaces::globalVars->curtime = simulationTime;
		interfaces::globalVars->frametime = interfaces::globalVars->interval_per_tick;

		self->m_iEFlags() |= static_cast<int>(EFlags::DIRTY_ABSVELOCITY);
		self->m_fEffects() |= static_cast<int>(EEffects::NOINTERP);

		bool ret = SetupBonesHook::Get().GetOriginal()(self, pBoneToWorldOut, nMaxBones, boneMask, simulationTime);

		self->m_fEffects() &= ~static_cast<int>(EEffects::NOINTERP);
		self->m_iEFlags() &= ~static_cast<int>(EFlags::DIRTY_ABSVELOCITY);

		interfaces::globalVars->curtime = oldCurtime;
		interfaces::globalVars->frametime = oldFrametime;

		return ret;
	}
}
