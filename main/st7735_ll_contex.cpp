#include <string.h>
#include "gui/ll_context.h"

inline static int XYToIndex(int x, int y) {
    return y * 128 + x;
}
inline static uint16_t ColorTransform(Color c) {
    return (((c.r & 0xF8) << 8) | ((c.g & 0xFC) << 3) | (c.b >> 3));
}

void LLContext::SetColor(int x, int y, Color c) {
    uint16_t* p = (uint16_t*)context_ + XYToIndex(x, y);
    *p = ColorTransform(c);
}

void LLContext::FillColorHorizenLine(int y, int x, int w, Color c) {
    uint16_t* p = (uint16_t*)context_ + XYToIndex(x, y);
    uint16_t color = ColorTransform(c);
    for (int i = 0; i < w; ++i) {
        *p++ = color;
    }
}

void LLContext::FillColorVeticalLine(int x, int y, int h, Color c) {
    uint16_t color = ColorTransform(c);
    for (int i = 0; i < h; ++i) {
        uint16_t* p = (uint16_t*)context_ + XYToIndex(x, y + i);
        *p = color;
    }
}

void LLContext::FillColorRect(int x, int y, int w, int h, Color c) {
    for (int i = 0; i < h; ++i) {
        FillColorHorizenLine(y + i, x, w, c);
    }
}