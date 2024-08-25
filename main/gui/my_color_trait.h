#pragma once

#include <stdint.h>
#include "color.h"

// some color type
using MyRGB888 = struct {};
using MyRGB565 = struct {};
using MyBGR565 = struct {};
using MyMono   = struct {};

// a color traits
template<typename T>
struct MyColorTraits {
    using type = void;
    static constexpr type ColorTransform(MyColor c) {
        return;
    }
};

template<>
struct MyColorTraits<MyRGB888> {
    using type = uint32_t;
    static constexpr auto kType = -1;
    static constexpr type ColorTransform(MyColor c) {
        return ((c.r & 0xFF) << 16) | ((c.g & 0xFF) << 8) | (c.b & 0xFF);
    }
};

template<>
struct MyColorTraits<MyRGB565> {
    using type = uint16_t;
    static constexpr auto kType = -2;
    static constexpr type ColorTransform(MyColor c) {
        return ((c.r & 0xF8) << 8) | ((c.g & 0xFC) << 3) | (c.b >> 3);
    }
};

template<>
struct MyColorTraits<MyBGR565> {
    using type = uint16_t;
    static constexpr auto kType = -3;
    static constexpr type ColorTransform(MyColor c) {
        return ((c.b & 0xF8) << 8) | ((c.g & 0xFC) << 3) | (c.r >> 3);
    }
};

template<>
struct MyColorTraits<MyMono> {
    using type = uint8_t;
    static constexpr auto kType = -4;
    static constexpr type ColorTransform(MyColor c) {
        return c.r | c.g | c.b;
    }
};