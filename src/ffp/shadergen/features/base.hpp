#pragma once

#include <GLES3/gl32.h>
#include <sstream>

namespace FFPE::Rendering::ShaderGen::Feature {

struct BaseFeature {

virtual ~BaseFeature() = default;

virtual void buildVS(
    [[maybe_unused]] std::stringstream& finalInputs,
    [[maybe_unused]] std::stringstream& finalOutputs,
    [[maybe_unused]] std::stringstream& finalOperations,
    [[maybe_unused]] std::stringstream& finalOutputOperations
) const { }

virtual void buildFS(
    [[maybe_unused]] std::stringstream& finalInputs,
    [[maybe_unused]] std::stringstream& finalOutputs,
    [[maybe_unused]] std::stringstream& finalOperations,
    [[maybe_unused]] std::stringstream& finalOutputOperations
) const { }

virtual void sendData([[maybe_unused]] GLuint program) const { }

virtual bool isEnabled() const { return false; }

};

}
