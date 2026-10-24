#pragma once

#include "common/defines.hpp"
#include "common/types.hpp"

#include <GLES3/gl32.h>

PUBLIC_API FunctionPtr glXGetProcAddress(const GLchar* pn);

void initDebug();
