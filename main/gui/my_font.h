#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <string_view>
#include <stdint.h>

class MyFont {
public:
    static constexpr int kMaxFontWidth = 64;

    int GetHeight() const {
        return 8;
    }

    int GetWidth(char c) const {
        return 8;
    }

    int GetMaxWidth() const {
        return 8;
    }

    int GetCharacterWidth(char c) const {
        return 8;
    }

    void FillCharacter(char c, uint8_t* mask) const;

    int GetTextWidth(std::string_view text) const {
        return 8 * text.size();
    }
};