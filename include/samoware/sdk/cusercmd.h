
#pragma once

#include "../crc32.h"

#include "angle.h"
#include "vector.h"

#include <cstring>

class CUserCmd {
public:
	enum ButtonFlags {
		IN_ATTACK	= 1 << 0,
		IN_JUMP		= 1 << 1,
		IN_DUCK		= 1 << 2,
		IN_FORWARD	= 1 << 3,
		IN_BACK		= 1 << 4,
		IN_USE		= 1 << 5,
		IN_CANCEL	= 1 << 6,
		IN_LEFT		= 1 << 7,
		IN_RIGHT	= 1 << 8,
		IN_MOVELEFT	= 1 << 9,
		IN_MOVERIGHT= 1 << 10,
		IN_ATTACK2	= 1 << 11,
		IN_RUN		= 1 << 12,
		IN_RELOAD	= 1 << 13,
		IN_ALT1		= 1 << 14,
		IN_ALT2		= 1 << 15,
		IN_SCORE	= 1 << 16,
		IN_SPEED	= 1 << 17,
		IN_WALK		= 1 << 18,
		IN_ZOOM		= 1 << 19,
		IN_WEAPON1	= 1 << 20,
		IN_WEAPON2	= 1 << 21,
		IN_BULLRUSH	= 1 << 22,
		IN_GRENADE1	= 1 << 23,
		IN_GRENADE2	= 1 << 24,
		IN_ATTACK3	= 1 << 25
	};

	enum Impulses {
		FLASHLIGHT = 100
	};

	CUserCmd() {
		Reset();
	}

	void Reset() {
		memset(this, 0, sizeof(CUserCmd));
	}

	std::uint32_t GetChecksum() {
		CRC32_t crc;

		crc32::init(&crc);

	#define CALC(var) crc32::processBuffer(&crc, &var, sizeof(var))

		CALC(command_number);
		CALC(tick_count);
		CALC(viewangles);
		CALC(forwardmove);
		CALC(sidemove);
		CALC(upmove);
		CALC(buttons);
		CALC(impulse);
		CALC(weaponselect);
		CALC(weaponsubtype);
		CALC(random_seed);
		CALC(mousedx);
		CALC(mousedy);
		CALC(unk);

	#undef CALC

		crc32::final(&crc);

		return crc;
	}

	bool HasButtons(int flags) {
		return buttons & flags;
	}

	void SetButtons(int flags) {
		buttons |= flags;
	}

	void UnsetButtons(int flags) {
		buttons &= ~flags;
	}

	int command_number;
	int tick_count;
	Angle viewangles;
	float forwardmove;
	float sidemove;
	float upmove;
	int buttons;
	unsigned char impulse;
	int weaponselect;
	int weaponsubtype;
	int random_seed;
	short mousedx;
	short mousedy;
	bool hasbeenpredicted;
	char pad_0001[5];
	bool unk;
	bool context_menu;
	Vector context_normal;
	bool istyping;
	char pad_0002[244];
};

class CVerifiedUserCmd {
public:
	CUserCmd cmd;
	CRC32_t crc;
};
