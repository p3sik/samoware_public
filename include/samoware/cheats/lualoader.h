
#pragma once

#include <stack>
#include <mutex>
#include <string>

#include "cfw/singleton.h"

#include "../interfaces.h"

class LuaLoader : public cfw::Singleton<LuaLoader> {
public:
	LuaLoader(token);

	void ProcessQueue();
	void AddToQueue(const std::string& script);

private:
	std::mutex _mutex;
	std::stack<std::string> _queue;
};
