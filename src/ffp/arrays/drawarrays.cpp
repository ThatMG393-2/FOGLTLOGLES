#include "ffp/arrays/arrays.hpp"
#include "ffp/manager/manager.hpp"
#include "ffp/shadergen/shadergen.hpp"
#include "ffp/vao/vao.hpp"
#include "fogltlogles.hpp"
#include "state/state.hpp"

#include <GLES3/gl32.h>

void FFPE::Rendering::Arrays::drawArrays(GLenum mode, GLint first, GLuint count) {
    if (trackedStates->currentlyUsedProgram == 0) {
        // immediately assume FFP
        SaveUsedProgram sup;

        auto buffer = Rendering::VAO::prepareVAOForRendering(count);
        auto renderingProgram = Rendering::ShaderGen::getCachedOrNewProgram(FFPE::States::Manager::getOrBuildState());
        CALL(glUseProgram, renderingProgram);
        Rendering::ShaderGen::Uniforms::setupInputsForRendering(renderingProgram);

        glDrawArrays(mode, first, count);

        return;
    }

    glDrawArrays(mode, first, count);
}
