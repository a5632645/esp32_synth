#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include "color.h"

class LLContext {
public:
    LLContext(void* context) : context_(context) {}

    void SetColor(int x, int y, Color c);
    void FillColorHorizenLine(int y, int x, int w, Color c);
    void FillColorVeticalLine(int x, int y, int h, Color c);
    void FillColorRect(int x, int y, int w, int h, Color c);
    void FlushScreen(int x, int y, int w, int h);
private:
    void* context_ = nullptr;
};
