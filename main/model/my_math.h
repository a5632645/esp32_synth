#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <cmath>

template<typename T>
inline static constexpr T MySin(T x) {
    return std::sin(x);
}

template<typename T>
inline static constexpr T MyCos(T x) {
    return std::cos(x);
}

template<typename T>
inline static constexpr T MyPitchToFreq(T pitch) {
    return std::exp2(pitch / 12.0f) * 8.1758f;
}
