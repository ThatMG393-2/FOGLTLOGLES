#pragma once

#include <string>

namespace FFPE::Rendering::ShaderGen::Common {

inline constexpr std::string_view VS_TEMPLATE = R"(#version 320 es
// FOGLTLOGLES ShaderGen : Vertex Shader

// feature inputs
{}

// default outputs
flat out lowp int vertexID;

// feature outputs
{}

void main() {{
    // feature operations
    {}

    // feature output operations
    {}
    
    vertexID = gl_VertexID;
}})";

inline constexpr std::string_view FS_TEMPLATE = R"(#version 320 es
precision mediump float;

// FOGLTLOGLES ShaderGen : Fragment Shader

// default inputs
flat in lowp int vertexID;

// feature inputs
{}

uniform sampler2D tex0;

// feature outputs
{}

void main() {{
    // feature operations
    {}

    color *= texture(tex0, vertexTexCoord.xy);

    // feature output operations
    {}
}})";

namespace Whitespaces {
inline constexpr std::string DOUBLE_NEWLINE = "\n\n";
inline constexpr std::string SINGLE_NEWLINE = "\n";

inline constexpr std::string DOUBLE_NEWLINE_TAB = "\n\n\t\t";
inline constexpr std::string SINGLE_NEWLINE_TAB = "\n\t\t";
}

}
