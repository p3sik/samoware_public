
#pragma once

#include "angle.h"

class CClientState {
public:
	int m_nDeltaTick;

	char pad_0001[1380];

	float		m_flLastServerTickTime;	// the timestamp of last message
	bool		insimulation;

	int			oldtickcount;			// previous tick
	float		m_tickRemainder;		// client copy of tick remainder
	float		m_frameTime;			// dt of the current frame

	int			lastoutgoingcommand;	// Sequence number of last outgoing command
	int			chokedcommands;			// number of choked commands
	int			last_command_ack;		// last command sequence number acknowledged by server
	int			command_ack;			// current command sequence acknowledged by server
	int			m_nSoundSequence;		// current processed reliable sound sequence number
	bool		ishltv;					// true if HLTV server/demo

	char		pad_0002[75];

	Angle		viewangles;
};
