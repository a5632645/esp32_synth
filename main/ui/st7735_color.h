#pragma once

#include "../gui/my_frame.h"

struct St7735Color {
    using type = uint16_t;
};

template<>
struct MyColorTraits<St7735Color> {
    using type = uint16_t;

    static constexpr type ColorTransform(MyColor c) {
        type cc = MyColorTraits<MyBGR565>::ColorTransform(c);
        return (cc << 8) | (cc >> 8);
    }
};