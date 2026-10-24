#include "ffp/matrix/matrix.hpp"

namespace FFPE::Rendering::Matrices {

void setCurrentMatrix(GLenum mode) {
    States::currentMatrixType = mode;
    States::currentMatrix = &States::matrices[mode];
}

void modifyCurrentMatrix(const std::function<glm::mat4(glm::mat4)>& newMatrix) {
    States::currentMatrix->matrix = newMatrix(States::currentMatrix->matrix);

    if (States::currentMatrixType == GL_PROJECTION
     || States::currentMatrixType == GL_MODELVIEW) MVP::invalidateMVP();
}

void pushCurrentMatrix() {
    States::currentMatrix->stack.push(States::currentMatrix->matrix);
}

void popTopMatrix() {
    if (States::currentMatrix->stack.empty()) return;

    States::currentMatrix->matrix = States::currentMatrix->stack.top();
    States::currentMatrix->stack.pop();

    if (States::currentMatrixType == GL_PROJECTION
     || States::currentMatrixType == GL_MODELVIEW) MVP::invalidateMVP();
}

const glm::mat4 MVP::getModelViewProjection() {
    if (!MVP::States::shouldRecalculate) return MVP::States::modelViewProjection;

    MVP::States::shouldRecalculate = false;
    return MVP::States::modelViewProjection = getMatrix(GL_PROJECTION).matrix * getMatrix(GL_MODELVIEW).matrix;
}

}
