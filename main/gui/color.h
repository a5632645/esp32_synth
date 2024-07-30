#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <cstdint>

struct MyColor {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    MyColor() = default;
    constexpr MyColor(uint8_t r, uint8_t g, uint8_t b)
        : r(r), g(g), b(b) {}
    constexpr MyColor(uint32_t data)
        : r((data >> 24) & 0xFF), g((data >> 16) & 0xFF), b((data >> 8) & 0xFF) {}
};

namespace MyColors {
constexpr auto kBlack = MyColor(0, 0, 0);
constexpr auto kWhite = MyColor(0xFF, 0xFF, 0xFF);
constexpr auto kRed = MyColor(0xFF, 0x00, 0x00);
constexpr auto kGreen = MyColor(0x00, 0xFF, 0x00);
constexpr auto kBlue = MyColor(0x00, 0x00, 0xFF);
}