#pragma once

#include "GLES3/gl32.h"

#include "utils/types.h"
#include "utils/defines.h"

PUBLIC_API FunctionPtr glXGetProcAddress(const GLchar* pn);

void initDebug();