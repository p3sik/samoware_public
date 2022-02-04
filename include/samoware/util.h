
#pragma once

#include "netvars.h"

#include <cstdint>
#include <assert.h>

#define NETVAR_(type, tableName, propName, funcName)										\
	type& funcName() const noexcept {														\
		static const int offset = netvars::netvars[#tableName "->" #propName];				\
		assert(offset != NULL);																\
		return *reinterpret_cast<type*>(reinterpret_cast<std::uintptr_t>(this) + offset);	\
	}

#define NETVAR(type, tblname, propname) NETVAR_(type, tblname, propname, propname)

#define OFFSETVAR(type, funcName, offset) type& funcName() const noexcept { return *reinterpret_cast<type*>(reinterpret_cast<std::uintptr_t>(this) + offset); }
