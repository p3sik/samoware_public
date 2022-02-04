
#pragma once

namespace hooks {
	namespace cl_move {
		typedef void(__stdcall* CL_MoveFn)(float, bool);
		extern CL_MoveFn CL_MoveOrig;

		extern bool hook();
		extern bool unhook();
	}
}
