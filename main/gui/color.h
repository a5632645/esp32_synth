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

    constexpr MyColor operator*(uint8_t alpha) const {
        return MyColor((r * alpha) >> 8, (g * alpha) >> 8, (b * alpha) >> 8);
    }


    constexpr uint16_t RGB565() const {
        return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
    }
    constexpr uint16_t BGR565() const {
        return ((b & 0xF8) << 8) | ((g & 0xFC) << 3) | (r >> 3);
    }
    constexpr uint32_t RGB888() const {
        return ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
    }
    constexpr uint8_t Mono() const {
        return (r | g | b) == 0 ? 1 : 0;
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