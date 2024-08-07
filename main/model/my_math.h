#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <cmath>

template<typename T>
inline static T MySin(T x) {
    return std::sin(x);
}

template<typename T>
inline static T MyCos(T x) {
    return std::cos(x);
}

template<typename T>
inline static T MyPitchToFreq(T pitch) {
    return std::exp2(pitch / 12.0f) * 8.1758f;
}
