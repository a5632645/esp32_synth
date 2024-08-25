#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <cstdint>
#include <stdlib.h>

struct MyColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    MyColor() = default;
    constexpr MyColor(uint8_t r, uint8_t g, uint8_t b)
        : r(r), g(g), b(b) {}
    constexpr MyColor(uint32_t data)
        : r((data >> 24) & 0xFF), g((data >> 16) & 0xFF), b((data >> 8) & 0xFF) {}

    constexpr MyColor operator*(uint8_t alpha) const {
        return MyColor((r * alpha) >> 8, (g * alpha) >> 8, (b * alpha) >> 8);
    }

    inline static MyColor RandomOne() {
        return MyColor(rand() % 256, rand() % 256, rand() % 256);
    }
};

namespace colors {
constexpr auto kBlack = MyColor(0, 0, 0);
constexpr auto kGrey = MyColor(0x80, 0x80, 0x80);
constexpr auto kWhite = MyColor(0xFF, 0xFF, 0xFF);
constexpr auto kRed = MyColor(0xFF, 0x00, 0x00);
constexpr auto kGreen = MyColor(0x00, 0xFF, 0x00);
constexpr auto kBlue = MyColor(0x00, 0x00, 0xFF);
constexpr auto kYellow = MyColor(0xFF, 0xFF, 0x00);
constexpr auto kOrange = MyColor(0xFF, 0x80, 0x00);
constexpr auto kCyan = MyColor(0x00, 0xFF, 0xFF);
constexpr auto kMagenta = MyColor(0xFF, 0x00, 0xFF);
constexpr auto kPurple = MyColor(0x80, 0x00, 0xFF);
constexpr auto kBrown = MyColor(0x80, 0x80, 0x00);
constexpr auto kPink = MyColor(0xFF, 0x80, 0x80);
}