
#pragma once

#include <type_traits>

namespace cfw {
	template <class Enum_t>
	class EnumHelper {
	private:
		using EnumValue_t = std::underlying_type_t<Enum_t>;
		EnumValue_t _value;

	public:
		EnumHelper() : _value(0) {};

		EnumHelper(EnumValue_t value) : _value(value) {};

		EnumHelper(Enum_t value) {
			_value = static_cast<EnumValue_t>(value);
		}

		EnumHelper& operator=(Enum_t value) {
			_value = static_cast<EnumValue_t>(value);
			return *this;
		}

		bool operator==(Enum_t value) {
			return static_cast<Enum_t>(_value) == value;
		}

		bool operator==(EnumValue_t value) {
			return value == value;
		}

		EnumValue_t* Ptr() {
			return &_value;
		}

		Enum_t& GetEnum() {
			return reinterpret_cast<Enum_t&>(_value);
		}

		EnumValue_t& GetValue() {
			return _value;
		}
	};
}
