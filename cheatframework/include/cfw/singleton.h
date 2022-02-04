
#pragma once

#include <Windows.h>
#include <stdio.h>
#include <typeinfo>

// https://codereview.stackexchange.com/questions/173929/modern-c-singleton-template
namespace cfw {
	template <class ParentClass_t>
	class Singleton {
	protected:
		struct token {};
		Singleton() {}

	public:
		Singleton(const Singleton&) = delete;
		Singleton& operator=(const Singleton) = delete;

		static ParentClass_t& Get() {
			static ParentClass_t instance {token{}};
			return instance;
		}
	};
}
