#pragma once

#include <string>

namespace FFPE::Rendering::ShaderGen::Common {

inline const std::string SG_NEWLINE = "\n\n";

inline const std::string VS_TEMPLATE = R"(#version 320 es
// FOGLTLOGLES ShaderGen : Vertex Shader

// default inputs

// vertex buffer data
layout(location = 0) in mediump vec4 iVertexPosition;
layout(location = 1) in lowp vec4 iVertexColor;
layout(location = 2) in mediump vec4 iVertexTexCoord;

// uniforms
uniform mat4 uModelViewProjection;

// default outputs
flat out lowp int vertexID;
out lowp vec4 vertexColor;
out mediump vec4 vertexTexCoord;

void main() {
    gl_Position = uModelViewProjection * iVertexPosition;

    vertexID = gl_VertexID;
    vertexColor = iVertexColor;
    vertexTexCoord = iVertexTexCoord;
})";

inline const std::string FS_TEMPLATE = R"(#version 320 es
precision mediump float;

// FOGLTLOGLES ShaderGen : Fragment Shader

// default inputs
flat in lowp int vertexID;
in lowp vec4 vertexColor;
in mediump vec4 vertexTexCoord;

// feature inputs
{}

// default output/s
out lowp vec4 oFragColor;

// feature outputs
{}

void main() {{
    lowp vec4 color = vertexColor;

    // feature operations
    {}

    oFragColor = color;
}})";

}
