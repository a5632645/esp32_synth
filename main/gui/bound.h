#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

struct Bound {
    int x_{};
    int y_{};
    int w_{};
    int h_{};

    Bound() = default;

    constexpr Bound(int x, int y, int w, int h)
        : x_(x), y_(y), w_(w), h_(h) {}

    constexpr int Top() const { return y_; }
    constexpr int Bottom() const { return y_ + h_; }
    constexpr int Left() const { return x_; }
    constexpr int Right() const { return x_ + w_; }

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

    constexpr Bound LimitIn(Bound bound) {
        auto x = std::max(x_, bound.x_);
        auto y = std::max(y_, bound.y_);
        auto w = std::min(x_ + w_, bound.x_ + bound.w_) - x;
        auto h = std::min(y_ + h_, bound.y_ + bound.h_) - y;
        return Bound{ x, y, w, h };
    }
};