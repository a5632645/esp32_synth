#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <string_view>

class MyFont {
public:
    static constexpr int kMaxFontWidth = 64;

    int GetHeight() const {
        return 8;
    }

    int GetWidth(char c) const {
        return 8;
    }

    void GetMask(char c, int offset_y, uint8_t* mask) const;
};