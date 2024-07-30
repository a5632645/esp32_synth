#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <algorithm>

struct Bound {
    int x_{};
    int y_{};
    int w_{};
    int h_{};

    Bound() = default;

    constexpr Bound(int x, int y, int w, int h)
        : x_(x), y_(y), w_(w), h_(h) {}

    constexpr bool operator==(const Bound& bound) const {
        return x_ == bound.x_ && y_ == bound.y_ && w_ == bound.w_ && h_ == bound.h_;
    }

    constexpr int Top() const { return y_; }
    constexpr int Bottom() const { return y_ + h_ - 1; }
    constexpr int Left() const { return x_; }
    constexpr int Right() const { return x_ + w_ - 1; }

    constexpr void Expaned(int dx, int dy) {
        x_ -= dx;
        y_ -= dy;
        w_ += 2 * dx;
        h_ += 2 * dy;
    }
    constexpr Bound Expand(int dx, int dy) {
        return Bound{ x_ - dx, y_ - dy, w_ + 2 * dx, h_ + 2 * dy };
    }

    constexpr void Shifted(int dx, int dy) {
        x_ += dx;
        y_ += dy;
    }
    constexpr Bound Shift(int dx, int dy) {
        return Bound{ x_ + dx, y_ + dy, w_, h_ };
    }

    constexpr Bound WithCenter(int w, int h) {
        return Bound{ x_ + w_ / 2 - w / 2, y_ + h_ / 2 - h / 2, w, h };
    }

    constexpr Bound GetIntersection(Bound bound) {
        auto b = GetIntersectionUncheck(bound);
        b.w_ = std::max(0, b.w_);
        b.h_ = std::max(0, b.h_);
        return b;
    }

    /** @brief get intersection of two bounds without check
     *  @return intersection, w and h may be negative
     */
    constexpr Bound GetIntersectionUncheck(Bound bound) {
        auto x = std::max(x_, bound.x_);
        auto y = std::max(y_, bound.y_);
        auto w = std::min(x_ + w_, bound.x_ + bound.w_) - x;
        auto h = std::min(y_ + h_, bound.y_ + bound.h_) - y;
        return Bound{ x, y, w, h };
    }

    constexpr bool IsValid() const { return w_ > 0 && h_ > 0; }

    constexpr bool Contain(Bound bound) const {
        return x_ <= bound.x_ && y_ <= bound.y_ && x_ + w_ >= bound.x_ + bound.w_ && y_ + h_ >= bound.y_ + bound.h_;
    }
};