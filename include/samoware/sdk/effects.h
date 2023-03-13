
#pragma once

#include "vector.h"
#include "angle.h"

struct CEffectData {
	Vector			m_vOrigin;
	Vector			m_vStart;
	Vector			m_vNormal;
	Angle			m_vAngles;
	int				m_fFlags;
	int				m_nEntIndex;
	float			m_flScale;
	float			m_flMagnitude;
	float			m_flRadius;
	int				m_nAttachmentIndex;
	short			m_nSurfaceProp;
	int				m_nMaterial;
	int				m_nDamageType;
	int				m_nHitBox;
	unsigned char	m_nColor;

};

typedef void(__cdecl* ClientEffectCallback)(const CEffectData& data);
class CClientEffectRegistration {
public:
	const char* m_pEffectName;
	ClientEffectCallback m_pFunction;
	CClientEffectRegistration* m_pNext;
};
