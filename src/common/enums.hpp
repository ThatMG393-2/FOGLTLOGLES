#pragma once

#include <cstdint>

typedef double   GLdouble;
typedef uint64_t GLbitfield64;

// types
#define GL_DOUBLE 0x140a

// States

// color
#define GL_COLOR_MATERIAL 0xb57

// FFP

// arrays
#define GL_COLOR_ARRAY         0x8076
#define GL_NORMAL_ARRAY        0x8075
#define GL_TEXTURE_COORD_ARRAY 0x8078

// alpha
#define GL_ALPHA_TEST 0xbc0

#define GL_NEVER    0x0200
#define GL_LESS     0x0201
#define GL_EQUAL    0x0202
#define GL_LEQUAL   0x0203
#define GL_GREATER  0x0204
#define GL_NOTEQUAL 0x0205
#define GL_GEQUAL   0x0206
#define GL_ALWAYS   0x0207

// shading
#define GL_FLAT   0x1d00
#define GL_SMOOTH 0x1d0

// fog
#define GL_FOG         0x0B60
#define GL_FOG_MODE    0x0B65
#define GL_FOG_COLOR   0x0B66
#define GL_FOG_START   0x0B63
#define GL_FOG_END     0x0B64
#define GL_FOG_DENSITY 0x0B62

#define GL_EXP  0x0800
#define GL_EXP2 0x0801

// lighting
#define GL_LIGHTING 0xb50

// draw array
// #define GL_QUADS 0x7

// Matrices
#define GL_MODELVIEW  0x1700
#define GL_PROJECTION 0x1701
#define GL_TEXTURE    0x1702

// Lists
#define GL_COMPILE             0x1300
#define GL_COMPILE_AND_EXECUTE 0x1301
