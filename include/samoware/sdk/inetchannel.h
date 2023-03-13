
#pragma once

#include <samoware/sdk/bitbuf.h>
#include <cfw/util.h>

class INetMessageHandler;

constexpr unsigned int NET_MESSAGE_BITS = 6;

enum class NetMessage {
	net_NOP = 0,
	net_Disconnect = 1,
	net_File = 2,
	net_Tick = 3,
	net_StringCmd = 4,
	net_SetConVar = 5,
	net_SignonState = 6,

	svc_Print = 7,
	svc_ServerInfo = 8,
	svc_SendTable = 9,
	svc_ClassInfo = 10,
	svc_SetPause = 11,
	svc_CreateStringTable = 12,
	svc_UpdateStringTable = 13,
	svc_VoiceInit = 14,
	svc_VoiceData = 15,
	svc_Sounds = 17,
	svc_SetView = 18,
	svc_FixAngle = 19,
	svc_CrosshairAngle = 20,
	svc_BSPDecal = 21,
	svc_UserMessage = 23,
	svc_EntityMessage = 24,
	svc_GameEvent = 25,
	svc_PacketEntities = 26,
	svc_TempEntities = 27,
	svc_Prefetch = 28,
	svc_Menu = 29,
	svc_GameEventList = 30,
	svc_GetCvarValue = 31,
	svc_CmdKeyValues = 32,
	svc_GMod_ServerToClient = 33,

	clc_ClientInfo = 8,
	clc_Move = 9,
	clc_VoiceData = 10,
	clc_BaselineAck = 11,
	clc_ListenEvents = 12,
	clc_RespondCvarValue = 13,
	clc_FileCRCCheck = 14,
	clc_SaveReplay = 15,
	clc_CmdKeyValues = 16,
	clc_FileMD5Check = 17,
	clc_GMod_ClientToServer = 18
};

class INetChannelInfo {
public:
	enum {
		GENERIC = 0,	// must be first and is default group
		LOCALPLAYER,	// bytes for local player entity update
		OTHERPLAYERS,	// bytes for other players update
		ENTITIES,		// all other entity bytes
		SOUNDS,			// game sounds
		EVENTS,			// event messages
		USERMESSAGES,	// user messages
		ENTMESSAGES,	// entity messages
		VOICE,			// voice data
		STRINGTABLE,	// a stringtable update
		MOVE,			// client move cmds
		STRINGCMD,		// string command
		SIGNON,			// various signondata
		TOTAL,			// must be last and is not a real group
	};

	virtual const char* GetName(void) const = 0;	// get channel name
	virtual const char* GetAddress(void) const = 0; // get channel IP address as string
	virtual float		GetTime(void) const = 0;	// current net time
	virtual float		GetTimeConnected(void) const = 0;	// get connection time in seconds
	virtual int			GetBufferSize(void) const = 0;	// netchannel packet history size
	virtual int			GetDataRate(void) const = 0; // send data rate in byte/sec

	virtual bool		IsLoopback(void) const = 0;	// true if loopback channel
	virtual bool		IsTimingOut(void) const = 0;	// true if timing out
	virtual bool		IsPlayback(void) const = 0;	// true if demo playback

	virtual float		GetLatency(int flow) const = 0;	 // current latency (RTT), more accurate but jittering
	virtual float		GetAvgLatency(int flow) const = 0; // average packet latency in seconds
	virtual float		GetAvgLoss(int flow) const = 0;	 // avg packet loss[0..1]
	virtual float		GetAvgChoke(int flow) const = 0;	 // avg packet choke[0..1]
	virtual float		GetAvgData(int flow) const = 0;	 // data flow in bytes/sec
	virtual float		GetAvgPackets(int flow) const = 0; // avg packets/sec
	virtual int			GetTotalData(int flow) const = 0;	 // total flow in/out in bytes
	virtual int			GetSequenceNr(int flow) const = 0;	// last send seq number
	virtual bool		IsValidPacket(int flow, int frame_number) const = 0; // true if packet was not lost/dropped/chocked/flushed
	virtual float		GetPacketTime(int flow, int frame_number) const = 0; // time when packet was send
	virtual int			GetPacketBytes(int flow, int frame_number, int group) const = 0; // group size of this packet
	virtual bool		GetStreamProgress(int flow, int* received, int* total) const = 0;  // TCP progress if transmitting
	virtual float		GetTimeSinceLastReceived(void) const = 0;	// get time since last recieved packet in seconds
	virtual	float		GetCommandInterpolationAmount(int flow, int frame_number) const = 0;
	virtual void		GetPacketResponseLatency(int flow, int frame_number, int* pnLatencyMsecs, int* pnChoke) const = 0;
	virtual void		GetRemoteFramerate(float* pflFrameTime, float* pflFrameTimeStdDeviation) const = 0;

	virtual float		GetTimeoutSeconds() const = 0;
};

class INetMessage;

class INetChannel : public INetChannelInfo {
public:
	// Reference breaks VPROXY macro somehow
	static constexpr int vIndex_SendNetMsg = 40;
	bool SendNetMsg(INetMessage& msg, bool bForceReliable = false, bool bVoice = false) noexcept {
		return cfw::vmt::call<bool, INetMessage&>((void*)this, vIndex_SendNetMsg, msg, bForceReliable, bVoice);
	}

	VPROXY(SetChoked, 45, void, (void));
	VPROXY(SendDatagram, 46, int, (bf_write* data), data);

public:
	int			m_ConnectionState;

	// last send outgoing sequence number
	int			m_nOutSequenceNr;
	// last received incoming sequnec number
	int			m_nInSequenceNr;
	// last received acknowledge outgoing sequnce number
	int			m_nOutSequenceNrAck;

	// state of outgoing reliable data (0/1) flip flop used for loss detection
	int			m_nOutReliableState;
	// state of incoming reliable data
	int			m_nInReliableState;

	// number of choked packets
	int			m_nChokedPackets;
	int			m_PacketDrop;

	uint8_t pad_0001[42];

	bf_write m_StreamUnreliable;
};

class INetMessage {
public:
	virtual	~INetMessage() {};

	// Use these to setup who can hear whose voice.
	// Pass in client indices (which are their ent indices - 1).

	virtual void	SetNetChannel(INetChannel* netchan) = 0; // netchannel this message is from/for
	virtual void	SetReliable(bool state) = 0;	// set to true if it's a reliable message

	virtual bool	Process(void) = 0; // calles the recently set handler to process this message

	virtual	bool	ReadFromBuffer(bf_read& buffer) = 0; // returns true if parsing was OK
	virtual	bool	WriteToBuffer(bf_write& buffer) = 0;	// returns true if writing was OK

	virtual bool	IsReliable(void) const = 0;  // true, if message needs reliable handling

	virtual int				GetType(void) const = 0; // returns module specific header tag eg svc_serverinfo
	virtual int				GetGroup(void) const = 0;	// returns net message group of this message
	virtual const char* GetName(void) const = 0;	// returns network message name, eg "svc_serverinfo"
	virtual INetChannel* GetNetChannel(void) const = 0;
	virtual const char* ToString(void) const = 0; // returns a human readable string about message content
};

class CNetMessage : public INetMessage {
public:
	CNetMessage() {
		m_bReliable = true;
		m_NetChannel = nullptr;
	}

	virtual ~CNetMessage() {};

	virtual int		GetGroup() const { return INetChannelInfo::GENERIC; }
	INetChannel*	GetNetChannel() const { return m_NetChannel; }

	virtual void	SetReliable(bool state) { m_bReliable = state; };
	virtual bool	IsReliable() const { return m_bReliable; };
	virtual void    SetNetChannel(INetChannel* netchan) { m_NetChannel = netchan; }
	virtual bool	Process() { return false; };	// no handler set

protected:
	bool				m_bReliable;	// true if message should be send reliable
	INetChannel*		m_NetChannel;	// netchannel this message is from/for
	INetMessageHandler*	m_pMessageHandler;
};

class NetMessageWriteable : public CNetMessage {
public:
	bf_write write;
	NetMessage type;

public:
	NetMessageWriteable(NetMessage msgType, uint8_t* buffer, int bufferSize) : type(msgType) {
		write.StartWriting(buffer, bufferSize);
	}

	virtual int GetType() const { return static_cast<int>(type); }
	virtual const char* GetName(void) const { return "samowaremsg"; }
	virtual const char* ToString(void) const { return "samowaremsg"; }

	virtual bool WriteToBuffer(bf_write& buffer) {
		buffer.WriteBits(write.m_pData, write.m_iCurBit);
		return true;
	}

	virtual bool ReadFromBuffer(bf_read& buffer) { return false; }
};



class IBaseNetMessage {
public:
	virtual ~IBaseNetMessage() {};
};

class CNetMessageImpl : public IBaseNetMessage {
public:
	CNetMessageImpl() {
		m_bReliable = true;
		m_NetChannel = NULL;
	}

	virtual ~CNetMessageImpl() {};

	bool m_bReliable;	// true if message should be send reliable
	INetChannel* m_NetChannel;	// netchannel this message is from/for
	INetMessageHandler* m_pMessageHandler;
};

class CLC_Move : public CNetMessageImpl {
public:
	CLC_Move() {
		m_bReliable = false;
	}

	void SetupVMT() {
		static void** vmt = reinterpret_cast<void**>(cfw::getAbsAddr(cfw::findPattern("engine.dll", "FF C0 03 F8 48 89 B4 24 E8 10 00 00 48 8D 05") + 12));
		*reinterpret_cast<void***>(this) = vmt;
	}

	int			m_nBackupCommands;
	int			m_nNewCommands;
	int			m_nLength;
	byte		pad_0001[24];
	bf_read		m_DataIn;
	bf_write	m_DataOut;
};

