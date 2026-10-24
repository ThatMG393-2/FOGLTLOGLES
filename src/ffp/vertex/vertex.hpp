#pragma once

#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"

#include <cstddef>

namespace FFPE::States::VertexData {
    template<int PC, typename P, int CC, typename C, int NC, typename N, int TCC, typename TC>
    struct VertexRepresentation {
        VertexRepresentation() = default;
        VertexRepresentation(
            glm::vec<PC, P> p, glm::vec<CC, C> c, glm::vec<NC, N> n, glm::vec<TCC, TC> tc
        ) : position(p), color(c), normal(n), texCoord(tc) { }

        glm::vec<PC, P> position = glm::vec4(0, 0, 0, 1);
        glm::vec<CC, C> color = glm::vec4(255, 255, 255, 255);
        glm::vec<NC, N> normal = glm::vec3(0.0f, 0.0f, 1.0f);
        glm::vec<TCC, TC> texCoord = glm::vec4(0, 0, 0, 0);
    };

    inline glm::vec4 position = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    inline glm::vec4 color = glm::vec4(1.0f);
    inline glm::vec3 normal = glm::vec3(0.0f, 0.0f, 1.0f);
    inline glm::vec4 texCoord = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // TODO: make a <texunit, texcoord> map

    template<typename SRC, typename DST>
    inline void set(SRC src, DST& to) {
        static_assert(
            SRC::length() <= DST::length(),
            "'src' components must be less than or equal to 'dst' components"
        );
        for (size_t i = 0; i < SRC::length(); ++i) to[i] = src[i];
    }
}

namespace FFP {
    void registerVertexFunctions();
    void registerColorFunctions();
    void registerTexCoordFunctions();
    void registerNormalFunctions();
}
