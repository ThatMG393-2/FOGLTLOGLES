#pragma once

#include "common/types.hpp"
#include <string>

namespace FOGLTLOGLES {
    void registerFunction(std::string, FunctionPtr);
    FunctionPtr getFunctionAddress(std::string);

    void init();
}

#ifndef TO_FUNCPTR
#define TO_FUNCPTR(func) reinterpret_cast<FunctionPtr>(func)
#endif

#ifndef REGISTER
#define REGISTER(func) FOGLTLOGLES::registerFunction(#func, TO_FUNCPTR(func))
#endif

#ifndef REGISTEROV
#define REGISTEROV(func) FOGLTLOGLES::registerFunction(#func, TO_FUNCPTR(OV_##func))
#endif

#ifndef REGISTERREDIR
#define REGISTERREDIR(name, target) FOGLTLOGLES::registerFunction(#name, TO_FUNCPTR(target))
#endif

#ifndef GET_OVFUNC
#define GET_OVFUNC(type, name) reinterpret_cast<type>(FOGLTLOGLES::getFunctionAddress(#name))
#endif