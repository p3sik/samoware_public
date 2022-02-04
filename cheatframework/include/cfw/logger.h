
#pragma once

#include "singleton.h"
#include "util.h"

#include <string.h>
#include <sstream>
#include <vector>
#include <cstdint>
#include <utility>
#include <mutex>

#undef ERROR

namespace cfw {
	#pragma warning(push)
	#pragma warning(disable : 26812)

	enum class LogLevel : uint8_t {
		DEBUG,
		INFO,
		WARNING,
		ERROR,
		SILENT
	};

	typedef void(__cdecl* LoggerCallbackFn)(LogLevel logLevel, std::string&& text);

	class Logger : public Singleton<Logger> {
	private:
		std::vector<std::pair<LogLevel, std::string>> _buffer;
		unsigned int _maxLines;
		LogLevel _logLevel;
		std::mutex _mutex;
		
		LoggerCallbackFn _callback;

	public:
		Logger(token) : _maxLines(100), _logLevel(LogLevel::INFO), _callback(nullptr) {};
		~Logger() {};

		void UpdateLines();

		LogLevel GetLogLevel() const;
		void SetLogLevel(LogLevel logLevel);

		void SetCallback(LoggerCallbackFn callback) {
			_callback = callback;
		}

		const char* LogLevelToString(LogLevel logLevel) const;

		const std::vector<std::pair<LogLevel, std::string>>& GetBuffer() const {
			return _buffer;
		}

		template <LogLevel logLevel = LogLevel::INFO, typename ...Args>
		void Log(Args&&... args) {
			if (static_cast<uint8_t>(_logLevel) > static_cast<uint8_t>(logLevel))
				return;

			std::stringstream unpackBuf;
			(unpackBuf << ... << std::forward<Args>(args));

			std::string text = unpackBuf.str();

			_mutex.lock();
			_buffer.push_back({logLevel, text});

			UpdateLines();
			_mutex.unlock();

			#ifdef _DEBUG
			std::string debugOutput = std::string("[") + LogLevelToString(logLevel) + std::string("] ") + text + "\n";
			OutputDebugStringA(debugOutput.c_str());
			#endif

			if (_callback)
				_callback(logLevel, std::move(text));
		}
	};

	#pragma warning(pop)
}
