#ifndef DEVILS_ENGINE_BINDINGS_NUKLEAR_H
#define DEVILS_ENGINE_BINDINGS_NUKLEAR_H

#include <cstddef>
#include <cstdint>
#include "lua_header.h"

namespace devils_engine {
namespace bindings {
void nk_functions(sol::table t);
}
}

#endif