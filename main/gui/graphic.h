#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <vector>
#include <cstdint>
#include "bound.h"
#include "color.h"
#include "ll_context.h"
#include <string_view>
#include "my_font.h"

class Graphic {
public:
    Graphic(LLContext& context)
        : buffer_bound_{context.GetBound()},
        context_(context) {}

    void SetClipBoundGlobal(Bound bound) {
         clip_bound_ = bound.GetIntersection(buffer_bound_); 
    }
    Bound GetClipBoundGlobal() const { return clip_bound_; }
    Bound GetClipBound() const { 
        return Bound{
            clip_bound_.x_ - component_bound_.x_,
            clip_bound_.y_ - component_bound_.y_,
            clip_bound_.w_, clip_bound_.h_
        }; 
    }
    void SetComponentBound(Bound bound) { component_bound_ = bound; }

    void SetColor(MyColor color) { color_ = color; }

    void Fill(MyColor color) {
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
        if (bound.Bottom() <= clip_bound_.Bottom())
            context_.FillColorHorizenLine(bound.Bottom(), limit_x, limit_w, color_);

        auto limit_y = std::max(0, bound.y_);
        auto limit_h = std::min(bound.h_, clip_bound_.h_ - limit_y);
        if (bound.x_ >= 0)
            context_.FillColorVeticalLine(bound.x_, limit_y, limit_h, color_);
        if (bound.Right() <= clip_bound_.Right())
            context_.FillColorVeticalLine(bound.Right(), limit_y, limit_h, color_);
    }
    void DrawRect(int x, int y, int w, int h) {
        DrawRect(Bound{x, y, w, h});
    }

    void DrawHorizenLine(int x, int y, int w) {
        y += component_bound_.y_;
        if (y < clip_bound_.Top() || y > clip_bound_.Bottom())
            return;
        x += component_bound_.x_;
        x = std::max(clip_bound_.x_, x);
        w = std::min(clip_bound_.w_ - x, w);
        context_.FillColorHorizenLine(y, x, w, color_);
    }
    
    void DrawVeticalLine(int x, int y, int h) {
        x += component_bound_.x_;
        if (x < clip_bound_.Left() || x > clip_bound_.Right())
            return;
        y += component_bound_.y_;
        y = std::max(clip_bound_.y_, y);
        h = std::min(clip_bound_.h_ - y, h);
        context_.FillColorVeticalLine(x, y, h, color_);
    }

    void DrawLine(int x1, int y1, int x2, int y2) {
        x1 += component_bound_.x_;
        y1 += component_bound_.y_;
        x2 += component_bound_.x_;
        y2 += component_bound_.y_;        
        x1 = std::max(clip_bound_.x_, x1);
        x1 = std::min(clip_bound_.Right(), x1);
        y1 = std::max(clip_bound_.y_, y1);
        y1 = std::min(clip_bound_.Bottom(), y1);
        x2 = std::max(clip_bound_.x_, x2);
        x2 = std::min(clip_bound_.Right(), x2);
        y2 = std::max(clip_bound_.y_, y2);
        y2 = std::min(clip_bound_.Bottom(), y2);

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

    void SetFont(MyFont font) {
        font_ = font;
    }

    void DrawSingleLineText(std::string_view text, int x, int y) {
        x += component_bound_.x_;
        y += component_bound_.y_;
        if (y + font_.GetHeight() <= clip_bound_.y_ || y >= clip_bound_.Bottom()) // no need to draw
            return;
        if (x > clip_bound_.Right()) // no need to draw
            return;

        // locate first character
        size_t i = 0;
        while (true) {
            int w = font_.GetWidth(text[i]);
            if (x < clip_bound_.x_ && x + w < clip_bound_.x_) {
                x += w;
                if (++i == text.size())
                    return;
            } else {
                break;
            }
        }

        text = text.substr(i);
        int left_x = x;
        int y_top = std::max(clip_bound_.y_, y);
        int y_bottom = std::min(clip_bound_.y_ + clip_bound_.h_, y + font_.GetHeight());
        uint8_t mask[MyFont::kMaxFontWidth] {};
        for (char c : text) {
            int right_x = left_x + font_.GetWidth(c);
            int real_left_x = std::max(clip_bound_.x_, left_x);
            int real_right_x = std::min(clip_bound_.x_ + clip_bound_.w_, right_x);
            int mask_x_offset = real_left_x - left_x;
            int mask_len = real_right_x - real_left_x;
            for (int j = y_top; j < y_bottom; ++j) {
                font_.GetMask(c, j - y, mask);
                context_.FillColorHorizenLineMask(j, real_left_x, mask_len, mask + mask_x_offset, color_);
            }

            left_x = right_x;
            if (left_x > clip_bound_.x_ + clip_bound_.w_)
                return;
        }
    }
private:
    Bound clip_bound_;
    Bound component_bound_;
    Bound buffer_bound_;
    MyColor color_;
    MyFont font_;
    LLContext& context_;
};