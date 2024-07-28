#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <vector>
#include <cstdint>
#include "bound.h"
#include "color.h"
#include "ll_context.h"

class Graphic {
public:
    Graphic(Bound buffer_bound,
            LLContext& context)
        : buffer_bound_(buffer_bound),
        context_(context) {}

    void SetTargetBound(Bound bound) { target_bound_ = bound; }

    void SetColor(Color color) { color_ = color; }

    void Fill(Color color) {
        auto old_co = color_;
        color_ = color;
        FillRect(target_bound_);
        color_ = old_co;
    }

    void FillRect(Bound bound) {
        bound = bound.Shift(target_bound_.x_, target_bound_.y_).LimitIn(buffer_bound_);
        context_.FillColorRect(bound.x_, bound.y_, bound.w_, bound.h_, color_);
    }
    void FillRect(int x, int y, int w, int h) {
        FillRect(Bound{x, y, w, h});
    }

    void DrawRect(Bound bound) {
        bound.Shifted(target_bound_.x_, target_bound_.y_);
        auto limit_x = std::max(0, bound.x_);
        auto limit_w = std::min(bound.w_, buffer_bound_.w_ - limit_x);
        if (bound.y_ >= 0)
            context_.FillColorHorizenLine(bound.y_, limit_x, limit_w, color_);
        if (bound.Bottom() < buffer_bound_.Bottom())
            context_.FillColorHorizenLine(bound.Bottom(), limit_x, limit_w, color_);

        auto limit_y = std::max(0, bound.y_);
        auto limit_h = std::min(bound.h_, buffer_bound_.h_ - limit_y);
        if (bound.x_ >= 0)
            context_.FillColorVeticalLine(bound.x_, limit_y, limit_h, color_);
        if (bound.Right() < buffer_bound_.Right())
            context_.FillColorVeticalLine(bound.Right(), limit_y, limit_h, color_);
    }
    void DrawRect(int x, int y, int w, int h) {
        DrawRect(Bound{x, y, w, h});
    }

    void DrawHorizenLine(int x, int y, int w) {
        y += target_bound_.y_;
        if (y < 0 || y >= buffer_bound_.h_)
            return;
        x += target_bound_.x_;
        x = std::max(0, x);
        w = std::min(buffer_bound_.w_ - x, w);
        context_.FillColorHorizenLine(y, x, w, color_);
    }
    
    void DrawVeticalLine(int x, int y, int h) {
        x += target_bound_.x_;
        if (x < 0 || x >= buffer_bound_.w_)
            return;
        y += target_bound_.y_;
        y = std::max(0, y);
        h = std::min(buffer_bound_.h_ - y, h);
        context_.FillColorVeticalLine(x, y, h, color_);
    }

    void DrawLine(int x1, int y1, int x2, int y2) {
        x1 += target_bound_.x_;
        y1 += target_bound_.y_;
        x2 += target_bound_.x_;
        y2 += target_bound_.y_;
        x1 = std::max(0, x1);
        y1 = std::max(0, y1);
        x2 = std::min(buffer_bound_.w_ - 1, x2);
        y2 = std::min(buffer_bound_.h_ - 1, y2);

        /* distance between two points */
        auto dx = ( x2 > x1 ) ? x2 - x1 : x1 - x2;
        auto dy = ( y2 > y1 ) ? y2 - y1 : y1 - y2;

        /* direction of two point */
        auto sx = ( x2 > x1 ) ? 1 : -1;
        auto sy = ( y2 > y1 ) ? 1 : -1;

        /* inclination < 1 */
        auto E = -dx;
        if ( dx > dy ) {
            E = -dx;
            for (int i = 0 ; i <= dx ; i++ ) {
                context_.SetColor(x1, y1, color_);
                x1 += sx;
                E += 2 * dy;
                if ( E >= 0 ) {
                y1 += sy;
                E -= 2 * dx;
            }
        }

    /* inclination >= 1 */
        } else {
            E = -dy;
            for (int i = 0 ; i <= dy ; i++ ) {
                context_.SetColor(x1, y1, color_);
                y1 += sy;
                E += 2 * dx;
                if ( E >= 0 ) {
                    x1 += sx;
                    E -= 2 * dy;
                }
            }
        }
    }
private:
    Bound target_bound_;
    Bound buffer_bound_;
    Color color_;
    LLContext& context_;
};