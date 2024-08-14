#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <cstdint>
#include "bound.h"
#include "color.h"
#include "my_frame.h"
#include <string_view>
#include "my_font.h"

// some code is from here, thanks to
// https://zingl.github.io/bresenham.html

enum class MyJustification {
    kLeft,
    kCenter,
    kRight
};

class MyGraphic {
public:
    MyGraphic(MyFrame* context)
        : clip_bound_(context->GetBound())
        , component_bound_{context->GetBound()}
        , buffer_bound_{context->GetBound()}
        , context_(context) {}

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

    // ================================================================================
    void SetColor(MyColor color) { context_->SetColor(color); }
    void DrawPoint(int16_t x, int16_t y, MyColor c) {
        if (clip_bound_.ContainPoint(x, y))
            context_->DrawPointColor(x, y, c); 
    }
    void DrawPoint(int16_t x, int16_t y) {
        if (clip_bound_.ContainPoint(x, y))
            context_->DrawPoint(x, y); 
    }
    void DrawPoint(MyPoint p) { DrawPoint(p.x_, p.y_); }
    void DrawPoint(MyPoint p, MyColor c) { DrawPoint(p.x_, p.y_, c); }

    // ================================================================================
    template<typename T>
    void DrawFrame(MyColoredFrame<T>& other, Bound mask, MyPoint pos) {
        auto b = mask.Shift(pos.x_, pos.y_);
        b = b.GetIntersectionUncheck(other.GetBound());
        if (!b.IsValid())
            return;

        b.Shifted(component_bound_.x_, component_bound_.y_);
        b = b.GetIntersectionUncheck(clip_bound_);
        if (!b.IsValid())
            return;

        auto& f = context_->As<MyColoredFrame<T>>();
        f.DrawFrameAt(other, b, MyPoint{pos.x_ + component_bound_.x_, pos.y_ + component_bound_.y_});
    }

    void Fill(MyColor color) {
        context_->FillRect(clip_bound_, color);
    }

    void FillRect(Bound bound, MyColor c) {
        bound = bound.Shift(component_bound_.x_, component_bound_.y_).GetIntersection(clip_bound_);
        context_->FillRect(bound, c);
    }
    void FillRect(int x, int y, int w, int h, MyColor c) {
        FillRect(Bound{x, y, w, h}, c);
    }

    void DrawRect(Bound bound);
    void DrawRect(int x, int y, int w, int h) {
        DrawRect(Bound{x, y, w, h});
    }

    void DrawHorizenLine(int x, int y, int w);
    void DrawHorizenLine2(int x1, int x2, int y) {
        if (x1 > x2)
            DrawHorizenLine(x2, y, x1 - x2);
        else
            DrawHorizenLine(x1, y, x2 - x1);
    }
    
    void DrawVeticalLine(int x, int y, int h);
    void DrawVeticalLine2(int y1, int y2, int x) {
        if (y1 > y2)
            DrawVeticalLine(x, y2, y1 - y2);
        else
            DrawVeticalLine(x, y1, y2 - y1);
    }

    void DrawLine(MyPoint p1, MyPoint p2) {
        DrawLine(p1.x_, p1.y_, p2.x_, p2.y_);
    }
    void DrawLine(int x1, int y1, int x2, int y2);

    void SetFont(MyFont font) {
        font_ = std::move(font);
    }
    const MyFont& GetFont() const {
        return font_;
    }

    /**
     * @brief draw single line text
     * @param text the text
     * @param x    the x of text left position
     * @param y    the y of text top position
     * @param w    the width of text, -1 means no limit
     */
    void DrawSingleLineText(std::string_view text, int x, int y, int w = -1);
    /**
     * @brief draw single line text with justification
     * @param text the text
     * @param x    the x of text left position
     * @param y    the y of text top position
     * @param w    the width of text, only in left justification can be -1!
     * @param j    the justification
     * @note  if you provide -1 width in center or right, nothing will happen
     */
    void DrawSingleLineText(std::string_view text, int x, int y, int w, MyJustification j);

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
    MyFont font_;
    MyFrame* context_;
};