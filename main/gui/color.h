#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <cstdint>

struct Color {
    uint8_t r;
    uint8_t g;
    uint8_t b;

    Color() = default;
    constexpr Color(uint8_t r, uint8_t g, uint8_t b)
        : r(r), g(g), b(b) {}
    constexpr Color(uint32_t data)
        : r((data >> 24) & 0xFF), g((data >> 16) & 0xFF), b((data >> 8) & 0xFF) {}
};

namespace colors {
constexpr auto kBlack = Color(0, 0, 0);
constexpr auto kWhite = Color(0xFF, 0xFF, 0xFF);
constexpr auto kRed = Color(0xFF, 0x00, 0x00);
constexpr auto kGreen = Color(0x00, 0xFF, 0x00);
constexpr auto kBlue = Color(0x00, 0x00, 0xFF);
}