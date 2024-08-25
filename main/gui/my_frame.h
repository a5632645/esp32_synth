#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <memory>
#include "color.h"
#include "bound.h"

class IMyRender;

// buffer are always y * w + x
struct MyFrame {
    MyFrame(void* buffer, int   color_type, int w, int h) 
        : buffer_(buffer), w_(w), h_(h), color_type_(color_type) {}

    Bound GetBound() const { return Bound{ 0, 0, w_, h_ }; }

    int GetColorType() const { return color_type_; }

    void* buffer_{};
    int color_type_;
    int w_ {};
    int h_ {};
};

extern std::unique_ptr<IMyRender> CreateMyRenderExtra(MyFrame& frame);
std::unique_ptr<IMyRender> CreateMyRender(MyFrame& frame);