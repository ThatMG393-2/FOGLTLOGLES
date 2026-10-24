#pragma once

#include "common/enums.hpp"

namespace FFPE::States::Manager {

namespace States {
    inline bool shouldRecalculate = true;
}

inline void invalidateCurrentState() {
    States::shouldRecalculate = true;
}

GLbitfield64 getOrBuildState();

}
