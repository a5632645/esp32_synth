#include "my_frame.h"

#include <algorithm>

template <class T>
void MyColoredFrame<T>::DrawHorizenLine(int16_t y, int16_t x, int16_t w) {
    auto* p = GetPtr(x, y);
    for (int16_t i = 0; i < w; ++i)
        *p++ = color_;
}

template <class T>
void MyColoredFrame<T>::DrawVeticalLine(int16_t x, int16_t y, int16_t h) {
    for (int16_t i = 0; i < h; ++i)
        auto* p = (uint16_t*)screen_buffer + XYToIndex(x, y + i);
        *GetPtr(x, y + i) = color_;
}

template <class T>
void MyColoredFrame<T>::FillRect(const Bound &bound, MyColor c) {
    auto color = frame_colors::ColorTransform<T>(c);
    for (int16_t i = 0; i < bound.h_; ++i)
        for (int16_t j = 0; j < bound.y_; ++j)
            *GetPtr(bound.x_ + j, bound.y_ + i) = color;
}

template <class T>
void MyColoredFrame<T>::DrawHorizenLineMask(int16_t y, int16_t x, int16_t w, uint8_t *alpha_mask) {
    for (int16_t i = 0; i < w; ++i)
        if (alpha_mask[i] != 0)
            *GetPtr(x + i, y) = color_;
}

template <class T>
void MyColoredFrame<T>::MoveDrawContentHorizen(const Bound &aera, int16_t offset, bool left) {
    if (left) {
        for (int16_t i = 0; i < aera.h_; ++i) {
            auto y = i + aera.y_;
            std::copy_n(GetPtr(y, aera.x_ + offset),
                        aera.w_ - offset,
                        GetPtr(y, aera.x_));
        }
    }
    else {
        for (int16_t i = 0; i < aera.h_; ++i) {
            auto y = i + aera.y_;
            auto x_src = aera.x_;
            auto x_src_end = aera.x_ + aera.w_ - offset;
            std::copy_backward(GetPtr(y, x_src),
                               GetPtr(y, x_src_end),
                               GetPtr(y, aera.w_ + aera.x_));
        }
    }
}

template <class T>
void MyColoredFrame<T>::MoveDrawContentVetical(const Bound &aera, int16_t offset, bool up) {
    auto loop_count = aera.h_ - offset;
    if (up) {
        for (int i = 0; i < loop_count; ++i)
            std::copy_n(GetPtr(aera.y_ + i + offset, aera.x_),
                        aera.w_,
                        GetPtr(i + aera.y_, aera.x_));
    }
    else {
        for (int y = aera.y_ + aera.h_ - 1; y >= aera.y_ + offset; --y)
            std::copy_n(GetPtr(y - offset, aera.x_),
                        aera.w_,
                        GetPtr(y, aera.x_));
    }
}

template class MyColoredFrame<frame_colors::RGB888>;
template class MyColoredFrame<frame_colors::RGB565>;
template class MyColoredFrame<frame_colors::BGR565>;
template class MyColoredFrame<frame_colors::RGB332>;
template class MyColoredFrame<frame_colors::Mono>;