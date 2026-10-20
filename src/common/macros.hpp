#pragma once

#include "common/types.hpp"

#ifndef TO_FUNCPTR
#define TO_FUNCPTR(f) reinterpret_cast<FunctionPtr>(f)
#endif