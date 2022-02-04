
#include "cfw/logger.h"

namespace cfw {
	void Logger::UpdateLines() {
		/*ptrdiff_t numLines = std::count(_buffer.begin(), _buffer.end(), '\n');

		if (numLines < _maxLines)
			return;

		size_t off = 0;
		for (ptrdiff_t i = 0; i < _maxLines; i++) {
			off = _buffer.rfind('\n', off);
		}

		_buffer.erase(0, off);*/
		
		if (_buffer.size() < _maxLines)
			return;

		_buffer.erase(_buffer.begin(), _buffer.begin() + min(_buffer.size(), _maxLines));
	}

	LogLevel Logger::GetLogLevel() const {
		return _logLevel;
	}

	void Logger::SetLogLevel(LogLevel logLevel) {
		_logLevel = logLevel;
	}

	const char* Logger::LogLevelToString(LogLevel logLevel) const {
		#define TOSTR(x) case LogLevel::x: return #x

		switch (logLevel) {
			TOSTR(DEBUG);
			TOSTR(INFO);
			TOSTR(WARNING);
			TOSTR(ERROR);
			TOSTR(SILENT);
		}

		#undef TOSTR

		return "UNKNOWN";
	}
}
