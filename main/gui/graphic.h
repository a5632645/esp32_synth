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

// some code is from here, thanks to
// https://zingl.github.io/bresenham.html
class Graphic {
public:
    Graphic(LLContext& context)
        : buffer_bound_{context.GetBound()},
        context_(context) {}

    void SetClipBoundGlobal(Bound bound) { // do not call this method!
         clip_bound_ = bound;
    }
    void SetClipBoundLocal(Bound bound) {
        clip_bound_ = bound.Shift(component_bound_.x_, component_bound_.y_).GetIntersection(buffer_bound_);
    }
    Bound GetClipBoundGlobal() const { return clip_bound_; }
    Bound GetClipBound() const { 
        return Bound{
            clip_bound_.x_ - component_bound_.x_,
            clip_bound_.y_ - component_bound_.y_,
            clip_bound_.w_, clip_bound_.h_
        }; 
    }
    void SetComponentBound(Bound bound) { component_bound_ = bound; } // do not call this method!

    void SetColor(MyColor color) { color_ = color; }

    void Fill(MyColor color) {
        context_.FillColorRect(clip_bound_, color);
    }

    void FillRect(Bound bound) {
        bound = bound.Shift(component_bound_.x_, component_bound_.y_).GetIntersection(clip_bound_);
        context_.FillColorRect(bound, color_);
    }
    void FillRect(int x, int y, int w, int h) {
        FillRect(Bound{x, y, w, h});
    }

    void DrawRect(Bound bound);
    void DrawRect(int x, int y, int w, int h) {
        DrawRect(Bound{x, y, w, h});
    }

    void DrawHorizenLine(int x, int y, int w);
    
    void DrawVeticalLine(int x, int y, int h);

    void DrawLine(MyPoint p1, MyPoint p2) {
        DrawLine(p1.x_, p1.y_, p2.x_, p2.y_);
    }
    void DrawLine(int x1, int y1, int x2, int y2);

    void SetFont(MyFont font) {
        font_ = font;
    }

    void DrawSingleLineText(std::string_view text, int x, int y, int w = -1);

    void DrawEllipse(Bound bound);
    void FillEllipe(Bound bound);

    void DrawTriangle(MyPoint p1, MyPoint p2, MyPoint p3) {
        DrawLine(p1, p2);
        DrawLine(p2, p3);
        DrawLine(p3, p1);
    }
    void FillTriangle(MyPoint p1, MyPoint p2, MyPoint p3) {
        // idk
        DrawTriangle(p1, p2, p3);
        auto p = p1 + p2 + p3;
        p.x_ /= 3;
        p.y_ /= 3;
        DrawSingleLineText("UNIMPLE", p.x_ - 3 * font_.GetWidth(' '), p.y_);
    }
    
    void DrawRoundRect(Bound b, int round_px) {
        // idk
        DrawRect(b);
        DrawSingleLineText("UNIMPLE", b.x_ + b.w_ / 2 - 3 * font_.GetWidth(' '), b.y_ + b.h_ / 2);
    }
    void FillRoundRect(Bound b, int round_px) {
        // idk
        DrawRoundRect(b, round_px);
    }

    void DrawArc(MyPoint center, int radius, int start_angle, int end_angle) {
        // idk
    }

    void SetPixel(MyPoint p) {
        if (clip_bound_.ContainPoint(p.x_, p.y_))
            context_.SetColor(p.x_, p.y_, color_);
    }

    void SetPixel(MyPoint p, MyColor c) {
        if (clip_bound_.ContainPoint(p.x_, p.y_))
            context_.SetColor(p.x_, p.y_, c);
    }

    /**
     * @brief move the draw content
     * @param aera       the aera you want to move
     * @param dx         horizontal offset, negative means left, positive means right
     * @param dy         vertical offset, negative means up, positive means down
     * @param background moved aera will be filled with background  
     */
    void MoveDrawContent(Bound aera, int dx, int dy, MyColor background);
private:
    Bound clip_bound_;
    Bound component_bound_;
    Bound buffer_bound_;
    MyColor color_;
    MyFont font_;
    LLContext& context_;
};