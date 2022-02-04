
#pragma once

#include "cfw/util.h"

typedef unsigned long HScheme;
typedef unsigned long long VPANEL;

class IPanel {
public:
	VPROXY(SetKeyBoardInputEnabled, 31, void, (VPANEL vguiPanel, bool state), vguiPanel, state);
	VPROXY(SetMouseInputEnabled, 32, void, (VPANEL vguiPanel, bool state), vguiPanel, state);
	VPROXY(GetName, 36, const char*, (VPANEL vguiPanel), vguiPanel);
	VPROXY(PaintTraverseFn, 41, void, (VPANEL vguiPanel, bool forceRepaint, bool allowForce), vguiPanel, forceRepaint, allowForce);
};
