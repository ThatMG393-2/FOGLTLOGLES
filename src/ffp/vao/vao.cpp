#include "ffp/vao/vao.hpp"
#include "common/es_utils.hpp"
#include "fogltlogles.hpp"

#include "glm/gtc/type_ptr.hpp"

#include <GLES3/gl32.h>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <type_traits>

namespace FFPE::Rendering::VAO {

void init() {
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vab);
}

namespace {

template<int PC, typename P, int CC, typename C, int NC, typename N, int TCC, typename TC>
using TypedVertexData = FFPE::States::VertexData::VertexRepresentation<PC, P, CC, C, NC, N, TCC, TC>;

template<typename F>
void mapVertexData(
    GLsizei count,
    States::ClientState::Arrays::ArrayState* vertex,
    States::ClientState::Arrays::ArrayState* color,
    States::ClientState::Arrays::ArrayState* normal,
    States::ClientState::Arrays::ArrayState* texCoord,
    const F&& callback
) {
    ESUtils::TypeTraits::typeToPrimitive(vertex->parameters.type, [&]<typename POS>() {
        ESUtils::TypeTraits::typeToPrimitive(color->parameters.type, [&]<typename COL>() {
            ESUtils::TypeTraits::typeToPrimitive(normal->parameters.type, [&]<typename NOR>() {
                ESUtils::TypeTraits::typeToPrimitive(texCoord->parameters.type, [&]<typename TEX>() {
                    using VertexData = TypedVertexData<4, POS, 4, COL, 3, NOR, 4, TEX>;
                    GLsizei newVABSize = count * sizeof(VertexData);

                    CALL(glBindBuffer, GL_ARRAY_BUFFER, vab);
                    CALL(glBufferData,
                        GL_ARRAY_BUFFER,
                        newVABSize,
                        nullptr,
                        GL_DYNAMIC_DRAW
                    );

                    auto* v = static_cast<VertexData*>(glMapBufferRange(
                        GL_ARRAY_BUFFER, 0,
                        newVABSize, GL_MAP_WRITE_BIT
                    ));

                    callback(v);

                    glUnmapBuffer(GL_ARRAY_BUFFER);
                });
            });
        });
    });
}

template<typename T1, typename T2>
void fillDataComponents(GLuint& offsetTracker, std::span<const T1> src, T2* dst) {
    for (size_t i = 0; i < src.size(); i++) (*dst)[i] = src[i];
    offsetTracker += src.size();
}

template<typename T, typename VD>
void putVertexDataInternal(GLenum arrayType, GLsizei componentSize, GLuint verticesCount, const T* src, VD* dst) {
    GLuint srcOffset = 0;

    switch (arrayType) {
        case GL_VERTEX_ARRAY:
            for (GLuint i = 0; i < verticesCount; ++i) {
                fillDataComponents(
                    srcOffset,
                    std::span(src + srcOffset, componentSize),
                    &dst[i].position
                );
            }
        break;

        case GL_COLOR_ARRAY:
            for (GLuint i = 0; i < verticesCount; ++i) {
                fillDataComponents(
                    srcOffset,
                    std::span(src + srcOffset, componentSize),
                    &dst[i].color
                );
            }
        break;

        case GL_TEXTURE_COORD_ARRAY:
            for (GLuint i = 0; i < verticesCount; ++i) {
                fillDataComponents(
                    srcOffset,
                    std::span(src + srcOffset, componentSize),
                    &dst[i].texCoord
                );
            }
        break;
    }
}

template<typename VD>
void putVertexData(GLenum arrayType, FFPE::States::ClientState::Arrays::ArrayState* array, VD* vertices, GLuint verticesCount) {
    ESUtils::TypeTraits::typeToPrimitive(array->parameters.type, [&]<typename T>() {
        putVertexDataInternal(
            arrayType, array->parameters.size, verticesCount,
            ESUtils::TypeTraits::asTypedArray<T>(
                array->parameters.firstElement
            ),
            vertices
        );
    });
}

// TODO: bind gl*Pointer here as VAO's

}

[[nodiscard]]
std::unique_ptr<SaveBoundedBuffer> prepareVAOForRendering(GLsizei count) {
    GLDebugGroup gldg("VAO preparations");

    auto* vertexArray = States::ClientState::Arrays::getArray(GL_VERTEX_ARRAY);
    auto* colorArray = States::ClientState::Arrays::getArray(GL_COLOR_ARRAY);
    auto* normalArray = States::ClientState::Arrays::getArray(GL_NORMAL_ARRAY);
    auto* texCoordArray = States::ClientState::Arrays::getTexCoordArray(GL_TEXTURE0);

    glBindVertexArray(vao);

    if (vertexArray->enabled) {
        glEnableVertexAttribArray(AttributeLocations::POSITION_LOCATION);
    } else {
        throw std::runtime_error("Vertex array not enabled! Don't know what to do!!");
    }

    if (colorArray->enabled) {
        glEnableVertexAttribArray(AttributeLocations::COLOR_LOCATION);
    } else {
        glDisableVertexAttribArray(AttributeLocations::COLOR_LOCATION);
        glVertexAttrib4fv(
            AttributeLocations::COLOR_LOCATION,
            glm::value_ptr(
                States::VertexData::color
            )
        );
    }

    if (normalArray->enabled) {
        glEnableVertexAttribArray(AttributeLocations::NORMAL_LOCATION);
    } else {
        glDisableVertexAttribArray(AttributeLocations::NORMAL_LOCATION);
        glVertexAttrib4fv(
            AttributeLocations::NORMAL_LOCATION,
            glm::value_ptr(
                States::VertexData::normal
            )
        );
    }

    if (texCoordArray->enabled) {
        glEnableVertexAttribArray(AttributeLocations::TEX_COORD_LOCATION);
    } else {
        glDisableVertexAttribArray(AttributeLocations::TEX_COORD_LOCATION);
        glVertexAttrib4fv(
            AttributeLocations::TEX_COORD_LOCATION,
            glm::value_ptr(
                States::VertexData::texCoord
            )
        );
    }

    std::unique_ptr<SaveBoundedBuffer> sbb;
    if (trackedStates->boundBuffers[GL_ARRAY_BUFFER].buffer == 0) {
        sbb = std::make_unique<SaveBoundedBuffer>(GL_ARRAY_BUFFER);

        if (!vertexArray->parameters.planar) {
            GLsizei newVABSize = count * vertexArray->parameters.stride;

            CALL(glBindBuffer, GL_ARRAY_BUFFER, vab);
            CALL(glBufferData,
                GL_ARRAY_BUFFER,
                newVABSize,
                vertexArray->parameters.firstElement,
                GL_DYNAMIC_DRAW
            );

            uintptr_t base = reinterpret_cast<uintptr_t>(vertexArray->parameters.firstElement);
            vertexArray->parameters = {
                vertexArray->parameters.planar,
                vertexArray->parameters.size, vertexArray->parameters.type,
                GL_FALSE, vertexArray->parameters.stride,
                nullptr
            };

            if (colorArray->enabled) {
                colorArray->parameters = {
                    colorArray->parameters.planar,
                    colorArray->parameters.size, colorArray->parameters.type,
                    GL_TRUE, colorArray->parameters.stride,
                    (void*) (reinterpret_cast<uintptr_t>(colorArray->parameters.firstElement) - base)
                };
            }

            if (normalArray->enabled) {
                normalArray->parameters = {
                    normalArray->parameters.planar,
                    normalArray->parameters.size, normalArray->parameters.type,
                    GL_FALSE, normalArray->parameters.stride,
                    (void*) (reinterpret_cast<uintptr_t>(normalArray->parameters.firstElement) - base)
                };
            }

            if (texCoordArray->enabled) {
                texCoordArray->parameters = {
                    texCoordArray->parameters.planar,
                    texCoordArray->parameters.size, texCoordArray->parameters.type,
                    GL_FALSE, texCoordArray->parameters.stride,
                    (void*) (reinterpret_cast<uintptr_t>(texCoordArray->parameters.firstElement) - base)
                };
            }

            goto buffered;
        }

        mapVertexData(
            count,
            vertexArray, colorArray, normalArray, texCoordArray,
            [&](auto* vertices) {
                using VertexData = std::remove_pointer_t<decltype(vertices)>;

                putVertexData(GL_VERTEX_ARRAY, vertexArray, vertices, count);
                vertexArray->parameters = {
                    vertexArray->parameters.planar,
                    decltype(VertexData::position)::length(),
                    vertexArray->parameters.type, GL_FALSE,
                    sizeof(VertexData),
                    (void*) offsetof(VertexData, position)
                };

                if (colorArray->enabled) {
                    putVertexData(GL_COLOR_ARRAY, colorArray, vertices, count);
                    colorArray->parameters = {
                        colorArray->parameters.planar,
                        decltype(VertexData::color)::length(),
                        colorArray->parameters.type, GL_TRUE,
                        sizeof(VertexData),
                        (void*) offsetof(VertexData, color)
                    };
                }

                if (normalArray->enabled) {
                    putVertexData(GL_NORMAL_ARRAY, normalArray, vertices, count);
                    normalArray->parameters = {
                        normalArray->parameters.planar,
                        decltype(VertexData::normal)::length(),
                        normalArray->parameters.type, GL_FALSE,
                        sizeof(VertexData),
                        (void*) offsetof(VertexData, normal)
                    };
                }

                if (texCoordArray->enabled) {
                    putVertexData(GL_TEXTURE_COORD_ARRAY, texCoordArray, vertices, count);
                    texCoordArray->parameters = {
                        texCoordArray->parameters.planar,
                        decltype(VertexData::texCoord)::length(),
                        texCoordArray->parameters.type, GL_FALSE,
                        sizeof(VertexData),
                        (void*) offsetof(VertexData, texCoord)
                    };
                }
            }
        );
    }

buffered:
    glVertexAttribPointer(
        AttributeLocations::POSITION_LOCATION,
        vertexArray->parameters.size, vertexArray->parameters.type,
        vertexArray->parameters.normalized,
        vertexArray->parameters.stride,
        vertexArray->parameters.firstElement
    );

    if (colorArray->enabled) {
        glVertexAttribPointer(
            AttributeLocations::COLOR_LOCATION,
            colorArray->parameters.size, colorArray->parameters.type,
            colorArray->parameters.normalized,
            colorArray->parameters.stride,
            colorArray->parameters.firstElement
        );
    }

    if (normalArray->enabled) {
        glVertexAttribPointer(
            AttributeLocations::NORMAL_LOCATION,
            normalArray->parameters.size, normalArray->parameters.type,
            normalArray->parameters.normalized,
            normalArray->parameters.stride,
            normalArray->parameters.firstElement
        );
    }

    if (texCoordArray->enabled) {
        glVertexAttribPointer(
            AttributeLocations::TEX_COORD_LOCATION,
            texCoordArray->parameters.size, texCoordArray->parameters.type,
            texCoordArray->parameters.normalized,
            texCoordArray->parameters.stride,
            texCoordArray->parameters.firstElement
        );
    }

    return sbb;
}

}
