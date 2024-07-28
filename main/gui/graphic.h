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

    void SetClipBound(Bound bound) { clip_bound_ = bound; }
    Bound GetClipBound() { return clip_bound_; }
    void SetComponentBound(Bound bound) { component_bound_ = bound; }

    void SetColor(Color color) { color_ = color; }

    void Fill(Color color) {
        context_.FillColorRect(clip_bound_.x_, clip_bound_.y_, clip_bound_.w_, clip_bound_.h_, color);
    }

    void FillRect(Bound bound) {
        bound = bound.Shift(component_bound_.x_, component_bound_.y_).GetIntersection(clip_bound_);
        context_.FillColorRect(bound.x_, bound.y_, bound.w_, bound.h_, color_);
    }
    void FillRect(int x, int y, int w, int h) {
        FillRect(Bound{x, y, w, h});
    }

    void DrawRect(Bound bound) {
        bound.Shifted(component_bound_.x_, component_bound_.y_);
        auto limit_x = std::max(clip_bound_.Left(), bound.x_);
        auto limit_w = std::min(bound.w_, clip_bound_.w_ - limit_x);
        if (bound.y_ >= 0)
            context_.FillColorHorizenLine(bound.y_, limit_x, limit_w, color_);
        if (bound.Bottom() < clip_bound_.Bottom())
            context_.FillColorHorizenLine(bound.Bottom(), limit_x, limit_w, color_);

        auto limit_y = std::max(0, bound.y_);
        auto limit_h = std::min(bound.h_, clip_bound_.h_ - limit_y);
        if (bound.x_ >= 0)
            context_.FillColorVeticalLine(bound.x_, limit_y, limit_h, color_);
        if (bound.Right() < buffer_bound_.Right())
            context_.FillColorVeticalLine(bound.Right(), limit_y, limit_h, color_);
    }
    void DrawRect(int x, int y, int w, int h) {
        DrawRect(Bound{x, y, w, h});
    }

    void DrawHorizenLine(int x, int y, int w) {
        y += clip_bound_.y_;
        if (y < clip_bound_.Top() || y >= clip_bound_.h_)
            return;
        x += clip_bound_.x_;
        x = std::max(0, x);
        w = std::min(clip_bound_.w_ - x, w);
        context_.FillColorHorizenLine(y, x, w, color_);
    }
    
    void DrawVeticalLine(int x, int y, int h) {
        x += clip_bound_.x_;
        if (x < clip_bound_.Left() || x >= clip_bound_.w_)
            return;
        y += clip_bound_.y_;
        y = std::max(0, y);
        h = std::min(clip_bound_.h_ - y, h);
        context_.FillColorVeticalLine(x, y, h, color_);
    }

    void DrawLine(int x1, int y1, int x2, int y2) {
        x1 += clip_bound_.x_;
        y1 += clip_bound_.y_;
        x2 += clip_bound_.x_;
        y2 += clip_bound_.y_;
        x1 = std::max(0, x1);
        y1 = std::max(0, y1);
        x2 = std::min(clip_bound_.w_ - 1, x2);
        y2 = std::min(clip_bound_.h_ - 1, y2);

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
    Bound clip_bound_;
    Bound component_bound_;
    Bound buffer_bound_;
    Color color_;
    LLContext& context_;
};