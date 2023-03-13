
#include "samoware/hooks/impacteffect.h"

#include "samoware/cheats/luaapi.h"

namespace hooks::impact {

ClientEffectCallback impactFunctionOriginal = nullptr;

void __cdecl ImpactFunctionHookFunc(const CEffectData& data) {
	luaapi::CallHook("OnImpact", 1, [&](ILuaBase* LUA) -> void {
		LUA->CreateTable();

		LUA->PushVector(data.m_vOrigin);
		LUA->SetField(-2, "m_vOrigin");

		LUA->PushVector(data.m_vStart);
		LUA->SetField(-2, "m_vStart");

		LUA->PushVector(data.m_vNormal);
		LUA->SetField(-2, "m_vNormal");

		LUA->PushAngle(data.m_vAngles);
		LUA->SetField(-2, "m_vAngles");

		LUA->PushNumber(data.m_nEntIndex);
		LUA->SetField(-2, "m_nEntIndex");

		LUA->PushNumber(data.m_nDamageType);
		LUA->SetField(-2, "m_nDamageType");

		LUA->PushNumber(data.m_nHitBox);
		LUA->SetField(-2, "m_nHitBox");
	 });

	impactFunctionOriginal(data);
}

void hookEffect(ClientEffectCallback hookFunc, const char* effectName) {
	static CClientEffectRegistration* s_pHead = nullptr;
	if (!s_pHead) {
		auto dispatchEffectToCallbackInst = cfw::findPattern("client.dll", "48 89 5C 24 30 48 8B 1D ?? ?? ?? ?? 48 85 DB");
		s_pHead = *reinterpret_cast<CClientEffectRegistration**>(cfw::getAbsAddr(dispatchEffectToCallbackInst + 0x5));
	}

	for (CClientEffectRegistration* pReg = s_pHead; pReg; pReg = pReg->m_pNext) {
		if (pReg->m_pEffectName && strcmp(pReg->m_pEffectName, effectName) == 0) {
			impactFunctionOriginal = pReg->m_pFunction;
			pReg->m_pFunction = hookFunc;
		}
	}
}

void hook() {
	hookEffect(ImpactFunctionHookFunc, "Impact");
}

void unHook() {
	hookEffect(impactFunctionOriginal, "Impact");
}

}
