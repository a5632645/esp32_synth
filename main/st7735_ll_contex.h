#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include "lcd.h"

struct St7735LLContext {
    static constexpr auto kWidth = 128;
    static constexpr auto kHeight = 160;

    ST7735_t dev;
    alignas(32) uint16_t screen_buffer[kWidth * kHeight] {};
};