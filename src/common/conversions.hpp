#pragma once

#include <cstdint>

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned char ubyte;

inline float clampf(const float f, const float min, const float max) {
  const float t = f < min ? min : f;
  return t > max ? max : t;
}

inline uint float_to_uint(const float x) {
    return static_cast<uint>(x);
}

inline float uint_to_float(const uint x) {
    return static_cast<float>(x);
}

inline float half_to_float(const ushort x) { // IEEE-754 16-bit floating-point format (without infinity): 1-5-10, exp-15, +-131008.0, +-6.1035156E-5, +-5.9604645E-8, 3.311 digits
    const uint e = (x & 0x7C00) >> 10; // exponent
    const uint m = (x & 0x03FF) << 13; // mantissa
    const uint v = float_to_uint((float) m) >> 23; // evil log2 bit hack to count leading zeros in denormalized format
    return uint_to_float((x & 0x8000) << 16
    			| (e != 0) * ((e+112) << 23 | m)
    			| ((e == 0) & (m != 0)) * ((v - 37) << 23
    			| ((m << (150 - v)) & 0x007FE000))); // sign : normalized : denormalized
}

inline ubyte float_to_ubyte(const float f) {
	return static_cast<ubyte>(clampf(f, 0.0f, 1.0f) * 255.0f);
}

inline uint32_t float_to_bits(float f) {
    uint32_t out;
    std::memcpy(&out, &f, sizeof(float));
    return out;
}
