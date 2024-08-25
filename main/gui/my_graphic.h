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
#include "my_render.h"

class MyGraphic {
public:
    MyGraphic(MyFrame& frame)
        : global_clip_bound_(frame.GetBound())
        , global_offset_({0, 0})
        , buffer_bound_(frame.GetBound())
        , render_(CreateMyRender(frame)) {}

    // do not call this method!
    void SetClipBoundGlobal(Bound bound) {
         global_clip_bound_ = bound;
    }

    void SetClipBoundLocal(Bound bound) {
        global_clip_bound_ = bound.Shift(global_offset_.x_, global_offset_.y_).GetIntersection(buffer_bound_);
    }

    Bound GetClipBoundGlobal() const { return global_clip_bound_; }

    Bound GetClipBound() const { 
        return Bound{
            global_clip_bound_.x_ - global_offset_.x_,
            global_clip_bound_.y_ - global_offset_.y_,
            global_clip_bound_.w_, global_clip_bound_.h_
        }; 
    }

    // do not call this method!
    void SetComponentBound(Bound bound) { 
        global_offset_ = {bound.x_, bound.y_}; 
    }

    // ================================================================================
    void SetColor(MyColor color) {
         color_ = color; 
    }

    void DrawPoint(int16_t x, int16_t y, MyColor c) {
        if (global_clip_bound_.ContainPoint(x, y))
            render_->DrawPixel(MyPoint{x, y}, c);
    }

    void DrawPoint(int16_t x, int16_t y) {
        DrawPoint(x, y, color_);
    }

    void DrawPoint(MyPoint p) {
        DrawPoint(p.x_, p.y_); 
    }
    void DrawPoint(MyPoint p, MyColor c) {
        DrawPoint(p.x_, p.y_, c); 
    }

    // ================================================================================
    void Fill(MyColor color) {
        IMyRender::RectReq req;
        req.aera = global_clip_bound_;
        req.clip = global_clip_bound_;
        req.c = color;
        render_->FillRect(req);
    }

    void FillRect(Bound bound) {
        IMyRender::RectReq req;
        req.aera = bound.Shift(global_offset_.x_, global_offset_.y_),
        req.c = color_;
        req.clip = global_clip_bound_;
        render_->FillRect(req);
    }

    void FillRect(int x, int y, int w, int h) {
        FillRect(Bound{x, y, w, h});
    }

    void DrawRect(Bound bound) {
        IMyRender::RectReq req;
        req.aera = bound.Shift(global_offset_.x_, global_offset_.y_),
        req.clip = global_clip_bound_;
        req.c = color_;
        render_->DrawRect(req);
    }

    void DrawRect(int x, int y, int w, int h) {
        DrawRect(Bound{x, y, w, h});
    }

    void DrawHorizenLine(int x, int y, int w) {
        IMyRender::LineReq req;
        req.start = MyPoint{x + global_offset_.x_, y + global_offset_.y_};
        req.end = MyPoint{x + w + global_offset_.x_, y + global_offset_.y_};
        req.c = color_;
        render_->DrawLine(req);
    }

    void DrawHorizenLine2(int x1, int x2, int y) {
        IMyRender::LineReq req;
        req.start = MyPoint{x1 + global_offset_.x_, y + global_offset_.y_};
        req.end = MyPoint{x2 + global_offset_.x_, y + global_offset_.y_};
        req.c = color_;
        render_->DrawLine(req);
    }
    
    void DrawVeticalLine(int x, int y, int h) {
        IMyRender::LineReq req;
        req.start = MyPoint{x + global_offset_.x_, y + global_offset_.y_};
        req.end = MyPoint{x + global_offset_.x_, y + h + global_offset_.y_};
        req.c = color_;
        render_->DrawLine(req);
    }
    void DrawVeticalLine2(int y1, int y2, int x) {
        IMyRender::LineReq req;
        req.start = MyPoint{x + global_offset_.x_, y1 + global_offset_.y_};
        req.end = MyPoint{x + global_offset_.x_, y2 + global_offset_.y_};
        req.c = color_;
        render_->DrawLine(req);
    }

    void DrawLine(MyPoint p1, MyPoint p2) {
        IMyRender::LineReq req;
        req.start = p1 + global_offset_;
        req.end = p2 + global_offset_;
        req.c = color_;
        render_->DrawLine(req);
    }
    void DrawLine(int x1, int y1, int x2, int y2) {
        DrawLine(MyPoint{x1, y1}, MyPoint{x2, y2});
    }

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
     */
    void DrawSingleLineText(std::string_view text, int x, int y) {
        IMyRender::TextReq req {
            .aera = global_clip_bound_,
            .c = color_,
            .font = font_,
            .t = text,
            .topleft = {x + global_offset_.x_, y + global_offset_.y_}
        };
        render_->DrawText(req);
    }
    /**
     * @brief draw single line text with justification
     * @param text the text
     * @param x    the x of text left position
     * @param y    the y of text top position
     * @param w    the width of text, only in left justification can be -1!
     * @param j    the justification
     * @note  if you provide -1 width in center or right, nothing will happen
     */
    void DrawAeraText(std::string_view text, Bound aera, MyTextAlignFlags my_text_align) {
        IMyRender::TextReq req {
            .aera = aera.Shift(global_offset_.x_, global_offset_.y_).GetIntersectionUncheck(global_clip_bound_),
            .c = color_,
            .font = font_,
            .t = text,
            .topleft = MyPoint{aera.x_, aera.y_} + global_offset_
        };

        if (my_text_align & MyTextAlign::kXCenter) {
            auto text_len = font_.GetTextWidth(text);
            req.topleft.x_ += (aera.w_ - text_len) / 2;
        }
        else if (my_text_align & MyTextAlign::kXRigh) {
            auto text_len = font_.GetTextWidth(text);
            req.topleft.x_ += (aera.w_ - text_len);
        }
        
        if (my_text_align & MyTextAlign::kYCenter) {
            auto text_height = font_.GetHeight();
            req.topleft.y_ += (aera.h_ - text_height) / 2;
        }
        else if (my_text_align & MyTextAlign::kYDown) {
            auto text_height = font_.GetHeight();
            req.topleft.y_ += (aera.h_ - text_height);
        }

        render_->DrawText(req);
    }

    void DrawMultiLineText(std::string_view text, Bound aera, MyTextAlignFlags my_text_align) {
        if (text.empty())
            return;

        std::vector<std::string_view> lines;
        size_t begin = 0;
        size_t end = 0;
        for (; end < text.size(); ++end) {
            if (text[end] != '\n') 
                continue;
            auto line = text.substr(begin, end - begin);
            begin = end + 1;
            if (!line.empty())
                lines.emplace_back(line);
        }
        auto t = text.substr(begin);
        if (!t.empty())
            lines.emplace_back(t);

        if (lines.empty())
            return;
        
        IMyRender::TextReq req {
            .aera = aera.Shift(global_offset_.x_, global_offset_.y_).GetIntersectionUncheck(global_clip_bound_),
            .c = color_,
            .font = font_,
            .t = text,
            .topleft = MyPoint{aera.x_, aera.y_} + global_offset_
        };
        
        auto height = font_.GetHeight() * lines.size();
        if (my_text_align & MyTextAlign::kYCenter) {
            req.topleft.y_ += (aera.h_ - height) / 2;
        }
        else if (my_text_align & MyTextAlign::kYDown) {
            req.topleft.y_ += (aera.h_ - height);
        }

        auto topleft = req.topleft.x_;
        for (auto line : lines) {
            if (my_text_align & MyTextAlign::kXCenter) {
                auto text_len = font_.GetTextWidth(line);
                req.topleft.x_ = (aera.w_ - text_len) / 2 + topleft;
            }
            else if (my_text_align & MyTextAlign::kXRigh) {
                auto text_len = font_.GetTextWidth(line);
                req.topleft.x_ = (aera.w_ - text_len) + topleft;
            }
            req.t = line;

            render_->DrawText(req);
            req.topleft.y_ += font_.GetHeight();
        }
    }

    void DrawEllipse(Bound bound) {
        IMyRender::EllipseReq req;
        req.aera = bound.Shift(global_offset_.x_, global_offset_.y_);
        req.clip = global_clip_bound_;
        req.c = color_;
        render_->DrawEllipse(req);
    }
    void FillEllipe(Bound bound) {
        IMyRender::EllipseReq req;
        req.aera = bound.Shift(global_offset_.x_, global_offset_.y_);
        req.clip = global_clip_bound_;
        req.c = color_;
        render_->FillEllipse(req);
    }

    void DrawTriangle(MyPoint p1, MyPoint p2, MyPoint p3) {
        MyPoint p[] = {p1, p2, p3};
        IMyRender::PathReq req;
        req.pdata = p;
        req.len = 3;
        req.c = color_;
        render_->DrawPath(req);
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
    void MoveDrawContent(Bound aera, int dx, int dy, MyColor background) {
        IMyRender::MoveContentReq req;
        req.aera = aera;
        req.dx = dx;
        req.dy = dy;
        render_->MoveDrawContent(req);
    }
private:
    MyColor color_;
    Bound global_clip_bound_;
    MyPoint global_offset_;
    Bound buffer_bound_;
    MyFont font_;
    std::unique_ptr<IMyRender> render_;
};