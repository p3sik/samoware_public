
#pragma once

#include "cutlvector.h"

struct VarMapEntry_t {
	unsigned short type;
	unsigned short m_bNeedsToInterpolate;

	void* data;
	void* watcher;
};

struct VarMapping_t {
	CUtlVector<VarMapEntry_t> m_Entries;
	int m_nInterpolatedEntries;
	float m_lastInterpolationTime;
};
