
#pragma once

#include <cfw/util.h>

class ClientClass;

enum class ClientFrameStage_t : int {
	FRAME_UNDEFINED = -1,
	FRAME_START,
	FRAME_NET_UPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_START,
	FRAME_NET_UPDATE_POSTDATAUPDATE_END,
	FRAME_NET_UPDATE_END,
	FRAME_RENDER_START,
	FRAME_RENDER_END
};

class bf_write;
class CHLClient {
public:
	VPROXY(GetAllClasses, 8, ClientClass*, (void));
	VPROXY(CreateMove, 21, void, (int sequence_number, float input_sample_frametime, bool active), sequence_number, input_sample_frametime, active);
	VPROXY(WriteUsercmdDeltaToBuffer, 23, bool, (bf_write* buf, int from, int to, bool isnewcommand), buf, from, to, isnewcommand);
	VPROXY(FrameStageNotify, 35, void, (ClientFrameStage_t stage), stage);
};
