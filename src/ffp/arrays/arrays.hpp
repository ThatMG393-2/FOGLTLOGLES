#pragma once

#include "state/state.hpp"

#include <GLES3/gl32.h>
#include <unordered_map>
#include <vector>

void glEnableClientState(GLenum array);
void glDisableClientState(GLenum array);

void glVertexPointer(GLint size, GLenum type, GLsizei stride, const void* pointer);
void glColorPointer(GLint size, GLenum type, GLsizei stride, const void* pointer);
void glNormalPointer(GLint size, GLenum type, GLsizei stride, const void* pointer);
void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const void* pointer);

namespace FFPE::States::ClientState {
    namespace Arrays {
        struct ArrayParameters {
            bool planar; // 1> = TRUE, !1 = !TRUE

            GLint size;
            GLenum type;
            bool normalized;
            GLsizei stride; // if stride == 0 then stride = size * sizeof(type) else stride = stride
            const void* firstElement;
        };

        struct ArrayState {
            bool enabled;
            ArrayParameters parameters;
        };

        inline std::unordered_map<GLenum, ArrayState> arrayStates;

        // GL_TEXTUREi, ArrayState
        inline std::unordered_map<GLenum, ArrayState> texCoordArrayStates;

        inline bool isArrayEnabled(GLenum array) {
            return arrayStates[array].enabled;
        }

        inline bool isTexCoordArrayEnabled(GLenum unit) {
            return texCoordArrayStates[unit].enabled;
        }

        inline ArrayState* getArray(GLenum array) {
            return &arrayStates[array];
        }

        inline ArrayState* getTexCoordArray(GLenum unit) {
            return &texCoordArrayStates[unit];
        }
    }

    inline GLenum currentTexCoordUnit;
    inline std::vector<GLenum> texCoordArrayTexUnits;
}

namespace FFPE::Rendering::Arrays {

namespace Quads {
    void init();
    GLuint generateEAB(GLuint count);
    void drawQuads(GLuint count);
}

void init();
void drawArrays(GLenum mode, GLint first, GLuint count);

}

class SaveClientArray : public Restorable<SaveClientArray> {
    friend class Restorable<SaveClientArray>;
    GLenum targetArray;
    bool isEnabled;

    void _internal_restore() {
        if (isEnabled) glEnableClientState(targetArray);
        else glDisableClientState(targetArray);
    }

public:
    explicit SaveClientArray(GLenum array)
        : targetArray(array),
          isEnabled(FFPE::States::ClientState::Arrays::getArray(array)->enabled) { }
    ~SaveClientArray() { restore(); }
};

class SaveTexCoordClientArray : public Restorable<SaveTexCoordClientArray> {
    friend class Restorable<SaveTexCoordClientArray>;
    GLenum targetUnit;
    bool isEnabled;

    void _internal_restore() {
        FFPE::States::ClientState::Arrays::getTexCoordArray(targetUnit)->enabled = isEnabled;
    }

public:
    explicit SaveTexCoordClientArray(GLenum unit)
        : targetUnit(unit),
          isEnabled(FFPE::States::ClientState::Arrays::getTexCoordArray(unit)->enabled) { }
    ~SaveTexCoordClientArray() { restore(); }
};

namespace FFP {
    void registerArrayFunctions();
    void registerTextureFunctions();
}
