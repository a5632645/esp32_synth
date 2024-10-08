#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <utility>
#include <algorithm>

struct MyPoint {
    int x_{};
    int y_{};

    MyPoint() = default;
    constexpr MyPoint(int x, int y) : x_(x), y_(y) {}
    constexpr bool operator==(const MyPoint& point) const {
        return x_ == point.x_ && y_ == point.y_;
    }

    constexpr MyPoint operator+(const MyPoint& point) const {
        return MyPoint{ x_ + point.x_, y_ + point.y_ };
    }
};

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

    constexpr Bound WithCenter(int x, int y) {
        auto c = GetCenter();
        auto xoffset = x - c.x_;
        auto yoffset = y - c.y_;
        return Shift(xoffset, yoffset);
    }

    constexpr Bound GetIntersection(Bound bound) const {
        auto b = GetIntersectionUncheck(bound);
        b.w_ = std::max(0, b.w_);
        b.h_ = std::max(0, b.h_);
        return b;
    }

    /** @brief get intersection of two bounds without check
     *  @return intersection, w and h may be negative
     */
    constexpr Bound GetIntersectionUncheck(Bound bound) const {
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

    constexpr bool ContainPoint(int x, int y) const {
        return x >= x_ && y >= y_ && x < x_ + w_ && y < y_ + h_;
    }

    constexpr MyPoint GetCenter() const {
        return MyPoint{ x_ + w_ / 2, y_ + h_ / 2 };
    }

    // ================================================================================
    constexpr Bound RemoveFromTop(int h) {
        auto b = Bound {x_, y_, w_, h};
        y_ += h;
        h_ -= h;
        return b;
    }
    constexpr Bound RemoveFromBottom(int h) {
        auto b = Bound{ x_, y_ + h_ - h, w_, h };
        h_ -= h;
        return b;
    }
    constexpr Bound RemoveFromLeft(int w) {
        auto b = Bound{ x_, y_, w, h_ };
        x_ += w;
        w_ -= w;
        return b;
    }
    constexpr Bound RemoveFromRight(int w) {
        auto b = Bound{ x_ + w_ - w, y_, w, h_ };
        w_ -= w;
        return b;
    }

    constexpr Bound WithWH(int w, int h) {
        return Bound{ x_, y_, w_, h_ };
    }
    constexpr Bound WithWidth(int w) {
        return Bound{ x_, y_, w, h_ };
    }
    constexpr Bound WithHeight(int h) {
        return Bound{ x_, y_, w_, h };
    }
    constexpr Bound WithXy(int x, int y) {
        return Bound{ x, y, w_, h_ };
    }
    constexpr Bound WithX(int x) {
        return Bound{ x, y_, w_, h_ };
    }
    constexpr Bound WithY(int y) {
        return Bound{ x_, y, w_, h_ };
    }
};