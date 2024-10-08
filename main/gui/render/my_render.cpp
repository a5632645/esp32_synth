#include "../my_render.h"
#include "my_color_trait.h"

#include <cmath>

// some code is from here, thanks to
// https://zingl.github.io/bresenham.html
template<typename ColorTrait>
class MyRender : public IMyRender {
public:
    using ColorStoreType = typename ColorTrait::StoreType;
    inline static auto ColorTransform(MyColor c) {
        return ColorTrait::ColorTransform(c);
    }
    inline static auto* GetXYPtr(int x, int y) {
        return static_cast<ColorStoreType*>(frame_.buffer_) + y * frame_.w_ + x;
    }

    void DrawPixel(MyPoint p, MyColor c) override;
    void DrawLine(const LineReq& req) override;
    void FillRect(const RectReq& req) override;
    void DrawRect(const RectReq& req) override;
    void DrawText(const TextReq& req) override;
    void DrawPath(const PathReq& req) override;
    void FillPath(const PathReq& req) override;
    void DrawFrame(const FrameReq& req) override;
    void DrawEllipse(const EllipseReq& req) override;
    void FillEllipse(const EllipseReq& req) override;
    void MoveDrawContent(const MoveContentReq& req) override;

private:
    void DrawVeticalline(const LineReq& req);
    void DrawHorizenLine(const LineReq& req);
};

template <typename ColorTrait>
void MyRender<ColorTrait>::DrawPixel(MyPoint p, MyColor c) {
    *GetXYPtr(p.x_, p.y_) = ColorTransform(c);
}

template <typename ColorTrait>
void MyRender<ColorTrait>::DrawLine(const LineReq& req) {
    if (req.start.x_ == req.end.x_) {
        DrawVeticalline(req);
    }
    else if (req.start.y_ == req.end.y_) {
        DrawHorizenLine(req);
    }
    else {
        auto c = ColorTransform(req.c);

        auto x1 = req.start.x_;
        auto y1 = req.start.y_;
        auto x2 = req.end.x_;
        auto y2 = req.end.y_;
        auto dx = ( x2 > x1 ) ? x2 - x1 : x1 - x2;
        auto dy = ( y2 > y1 ) ? y2 - y1 : y1 - y2;

        /* direction of two point */
        auto sx = ( x2 > x1 ) ? 1 : -1;
        auto sy = ( y2 > y1 ) ? 1 : -1;
        
        /* inclination < 1 */
        auto E = -dx;
        if ( dx > dy ) {
            E = -dx;
            for (int i = 0; i <= dx; i++) {
                if (req.aera.ContainPoint(x1, y1))
                    *GetXYPtr(x1, y1) = c;

                x1 += sx;
                E += 2 * dy;
                if (E >= 0) {
                    y1 += sy;
                    E -= 2 * dx;
                }
            }
        }
        else { /* inclination >= 1 */
            E = -dy;
            for (int i = 0 ; i <= dy ; i++ ) {
                if (req.aera.ContainPoint(x1, y1))
                    *GetXYPtr(x1, y1) = c;
                
                y1 += sy;
                    E += 2 * dx;
                if ( E >= 0 ) {
                    x1 += sx;
                    E -= 2 * dy;
                }
            }
        }
    }
}

template <typename ColorTrait>
void MyRender<ColorTrait>::DrawVeticalline(const LineReq& req) {
    auto x = req.start.x_;
    auto y = std::min(req.start.y_, req.end.y_);
    auto y_end = std::max(req.start.y_, req.end.y_);
    y = std::max(y, req.aera.y_);
    y_end = std::min(y_end, req.aera.y_ + req.aera.h_ - 1);
    auto c = ColorTransform(req.c);
    for (; y <= y_end; ++y) {
        *GetXYPtr(x, y) = c;
    }
}

template <typename ColorTrait>
void MyRender<ColorTrait>::DrawHorizenLine(const LineReq& req) {
    auto y = req.start.y_;
    auto x = std::min(req.start.x_, req.end.x_);
    auto x_end = std::max(req.start.x_, req.end.x_);
    x = std::max(x, req.aera.x_);
    x_end = std::min(x_end, req.aera.x_ + req.aera.w_);
    auto len = x_end - x;
    auto c = ColorTransform(req.c);
    std::fill_n(GetXYPtr(x, y), len, c);
}

template <typename ColorTrait>
void MyRender<ColorTrait>::FillRect(const RectReq& req) {
    auto c = ColorTransform(req.c);
    for (int y = req.y_;
         y < req.y_ + req.h_;
         ++y) {
        std::fill_n(GetXYPtr(req.x_, y), req.w_, c);
    }
}

template <typename ColorTrait>
void MyRender<ColorTrait>::DrawRect(const RectReq& req) {
    auto c = ColorTransform(req.c);
    std::fill_n(GetXYPtr(req.aera.x_, req.aera.y_), req.aera.w_, c);
    std::fill_n(GetXYPtr(req.aera.x_, req.aera.y_ + req.aera.h_ - 1), req.aera.w_, c);
    for (int x = req.aera.x_;
         x < req.aera.x_ + req.aera.w_;
         ++x) {
            *GetXYPtr(x, req.aera.y_) = c;
            *GetXYPtr(x, req.aera.y_ + req.aera.h_ - 1) = c;
    }
}

template <typename ColorTrait>
void MyRender<ColorTrait>::DrawText(const TextReq& req) {
    if (!req.aera.IsValid())
        return;
    if (req.topleft.y_ + req.font.GetHeight() < req.aera.y_)
        return;

    auto canvas_x = req.topleft.x_;
    auto canvas_y = req.topleft.y_;
    auto clip_left = req.aera.x_;
    size_t i = 0;
    for (; i < req.t.size(); ++i) {
        auto text_right = canvas_x + font_.GetCharacterWidth(text[i]);
        if (text_right >= clip_left)
            break;
        canvas_x = text_right;
    }
    if (i == req.t.size())
        return;

    MyPoint topleft_offset{canvas_x - req.aera.x_, canvas_y - req.aera.y_};
    auto c = ColorTransform(req.c);
    auto buffer_size = req.font.GetHeight() * req.font.GetMaxWidth();
    uint8_t* mask = alloca(buffer_size * sizeof(uint8_t));

    auto aera_bottom = req.aera.y_ + req.aera.h_;
    auto aera_right = req.aera.x_ + req.aera.w_;

    auto mask_y_offset = -req.topleft_offset.y_;
    auto mask_y_height = std::min(req.aera.h_, req.font.GetHeight() + req.topleft_offset.y_);
    auto mask_x_offset = -req.topleft_offset.x_;
    auto canvas_x = req.aera.x_;
    for (auto character : req.t) {
        if (canvas_x >= aera_right)
            break;

        auto chara_len = req.font.GetCharacterWidth(character);
        auto fill_len = std::min(chara_len, aera_right - canvas_x);
        req.font.FillCharacter(character, mask);
        for (int yy = 0; yy < mask_y_height; ++yy) {
            auto* canvas_ptr = GetXYPtr(canvas_x, req.aera.y_ + yy);
            auto* mask_ptr = mask + (mask_y_offset + yy) * req.font.GetMaxWidth() + mask_x_offset;
            for (int xx = 0; xx < fill_len; ++xx) {
                if (mask_ptr[xx] != 0)
                    canvas_ptr[xx] = c;
            }
        }

        canvas_x += chara_len;
        mask_x_offset = 0;
    }
}

template <typename ColorTrait>
void MyRender<ColorTrait>::DrawPath(const PathReq& req) {
    if (req.size < 2)
        return;

    LineReq lreq;
    lreq.start = req.points[0];
    lreq.c = req.c;
    for (size_t i = 1; i < req.len; ++i) {
        lreq.end = req.pdata[i];
        DrawLine(lreq);
        lreq.start = lreq.end;
    }
}

template <typename ColorTrait>
void MyRender<ColorTrait>::FillPath(const PathReq& req) {
}

template <typename ColorTrait>
void MyRender<ColorTrait>::DrawFrame(const FrameReq& req) {
    
}

template <typename ColorTrait>
void MyRender<ColorTrait>::DrawEllipse(const EllipseReq& req) {
    if (!req.aera.IsValid())
        return;

    auto c = ColorTransform(req.c);

    auto x0 = req.aera.Left();
    auto x1 = req.aera.Right();
    auto y0 = req.aera.Bottom();
    auto y1 = req.aera.Top();

    int a = abs(x1 - x0), b = abs(y1 - y0), b1 = b & 1;       /* values of diameter */
    long dx = 4 * (1 - a) * b * b, dy = 4 * (b1 + 1) * a * a; /* error increment */
    long err = dx + dy + b1 * a * a, e2;                      /* error of 1.step */

    if (x0 > x1) {
        x0 = x1;
        x1 += a;
    } /* if called with swapped points */
    if (y0 > y1)
        y0 = y1; /* .. exchange them */
    y0 += (b + 1) / 2;
    y1 = y0 - b1; /* starting pixel */
    a *= 8 * a;
    b1 = 8 * b * b;

    do {
        if (req.aera.ContainPoint(x1, y0))
            GetXYPtr(x1, y0) = c;
        if (req.aera.ContainPoint(x0, y0))
            GetXYPtr(x0, y0) = c;
        if (req.aera.ContainPoint(x0, y1))
            GetXYPtr(x0, y1) = c;
        if (req.aera.ContainPoint(x1, y1))
            GetXYPtr(x1, y1) = c;
        e2 = 2 * err;
        if (e2 <= dy) {
            y0++;
            y1--;
            err += dy += a;
        } /* y step */
        if (e2 >= dx || 2 * err > dy) {
            x0++;
            x1--;
            err += dx += b1;
        } /* x step */
    } while (x0 <= x1);

    while (y0 - y1 < b) {                         /* too early stop of flat ellipses a=1 */
        if (req.aera.ContainPoint(x0 - 1, y0))
            GetXYPtr(x0 - 1, y0) = c;
        if (req.aera.ContainPoint(x1 + 1, y0))
            GetXYPtr(x1 + 1, y0) = c;
        if (req.aera.ContainPoint(x0 - 1, y1))
            GetXYPtr(x0 - 1, y1) = c;
        if (req.aera.ContainPoint(x1 + 1, y1))
            GetXYPtr(x1 + 1, y1) = c;
    }
}

template <typename ColorTrait>
void MyRender<ColorTrait>::FillEllipse(const EllipseReq& req) {
    if (!req.aera.IsValid())
        return;

    auto center = req.aera.GetCenter();
    auto top_y = std::max(req.aera.y_, req.aera.y_);
    auto bottom_y = std::min(req.aera.y_ + req.aera.h_, req.aera.y_ + req.aera.h_);
    auto y_it_begin = top_y - center.y_;
    auto y_it_end = bottom_y - center.y_;
    auto a2 = static_cast<float>(req.aera.w_ * req.aera.w_ * 0.25f);
    auto b2 = static_cast<float>(req.aera.h_ * req.aera.h_ * 0.25f);
    auto div = a2 / b2;
    for (auto y = y_it_begin; y < y_it_end; ++y) {
        auto x2 = a2 - div * y * y;
        auto dx = std::sqrt(x2);
        auto left = static_cast<int>(center.x_ - dx);
        auto right = static_cast<int>(center.x_ + dx);
        left = std::max(left, req.aera.x_);
        right = std::min(right, req.aera.x_ + req.aera.w_);

        LineReq lq;
        lq.c = req.c;
        lq.start = {left, y + center.y_};
        lq.end = {right, y + center.y_};
        DrawHorizenLine(lq);
    }
}

template <typename ColorTrait>
void MyRender<ColorTrait>::MoveDrawContent(const MoveContentReq& req) {
    bool left = req.dx < 0;
    bool up = req.dy < 0;
    
    if (left) {
        for (int i = 0; i < req.aera.h_; ++i) {
            auto y = i + req.aera.y_;
            std::copy_n(GetPtr(y, req.aera.x_ + offset),
                        req.aera.w_ - offset,
                        GetPtr(y, req.aera.x_));
        }
    }
    else {
        for (int i = 0; i < req.aera.h_; ++i) {
            auto y = i + req.aera.y_;
            auto x_src = req.aera.x_;
            auto x_src_end = req.aera.x_ + req.aera.w_ - offset;
            std::copy_backward(GetPtr(y, x_src),
                               GetPtr(y, x_src_end),
                               GetPtr(y, req.aera.w_ + req.aera.x_));
        }
    }

    auto loop_count = req.aera.h_ - offset;
    if (up) {
        for (int i = 0; i < loop_count; ++i)
            std::copy_n(GetPtr(req.aera.y_ + i + offset, req.aera.x_),
                        req.aera.w_,
                        GetPtr(i + req.aera.y_, req.aera.x_));
    }
    else {
        for (int y = req.aera.y_ + req.aera.h_ - 1; y >= req.aera.y_ + offset; --y)
            std::copy_n(GetPtr(y - offset, req.aera.x_),
                        req.aera.w_,
                        GetPtr(y, req.aera.x_));
    }
}
