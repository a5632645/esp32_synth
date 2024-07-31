#include "graphic.h"

#include <cmath>

void Graphic::DrawRect(Bound bound) {
    bound.Shifted(component_bound_.x_, component_bound_.y_);
    auto limit_x = std::max(clip_bound_.x_, bound.x_);
    auto limit_w = std::min(bound.w_ + bound.x_, clip_bound_.w_ + clip_bound_.x_) - limit_x;
    if (bound.y_ >= clip_bound_.y_)
        context_.FillColorHorizenLine(bound.y_, limit_x, limit_w, color_);
    if (bound.Bottom() <= clip_bound_.Bottom())
        context_.FillColorHorizenLine(bound.Bottom(), limit_x, limit_w, color_);

    auto limit_y = std::max(0, bound.y_);
    auto limit_h = std::min(bound.h_ + bound.y_, clip_bound_.h_ + clip_bound_.y_) - limit_y;
    if (bound.x_ >= clip_bound_.x_)
        context_.FillColorVeticalLine(bound.x_, limit_y, limit_h, color_);
    if (bound.Right() <= clip_bound_.Right())
        context_.FillColorVeticalLine(bound.Right(), limit_y, limit_h, color_);
}

void Graphic::DrawHorizenLine(int x, int y, int w) {
    y += component_bound_.y_;
    if (y < clip_bound_.Top() || y > clip_bound_.Bottom())
        return;
    x += component_bound_.x_;
    auto real_x = std::max(clip_bound_.x_, x);
    auto real_w = std::min(clip_bound_.w_ + clip_bound_.x_, w + x) - real_x;
    context_.FillColorHorizenLine(y, real_x, real_w, color_);
}

void Graphic::DrawVeticalLine(int x, int y, int h) {
    x += component_bound_.x_;
    if (x < clip_bound_.Left() || x > clip_bound_.Right())
        return;
    y += component_bound_.y_;
    auto real_y = std::max(clip_bound_.y_, y);
    auto real_h = std::min(clip_bound_.h_ + clip_bound_.y_, h + y) - real_y;
    context_.FillColorVeticalLine(x, real_y, real_h, color_);
}

void Graphic::DrawLine(int x1, int y1, int x2, int y2) {
    x1 += component_bound_.x_;
    y1 += component_bound_.y_;
    x2 += component_bound_.x_;
    y2 += component_bound_.y_;

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
        for (int i = 0; i <= dx; i++) {
            if (clip_bound_.ContainPoint(x1, y1))
                context_.SetColor(x1, y1, color_);

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
            if (clip_bound_.ContainPoint(x1, y1))
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

void Graphic::DrawSingleLineText(std::string_view text, int x, int y, int w) { // TODO: justification
    if (text.empty())
        return;

    x += component_bound_.x_;
    y += component_bound_.y_;
    if (y + font_.GetHeight() <= clip_bound_.y_ || y > clip_bound_.Bottom()) // no need to draw
        return;

    auto clip_b = clip_bound_;
    if (w != -1) { // reset clip bound width if has width limit
        auto right = std::min(clip_bound_.x_ + clip_bound_.w_, x + w);
        clip_b.w_ = right - clip_b.x_;
    }

    if (x > clip_b.Right()) // no need to draw
        return;

    // locate first character
    size_t i = 0;
    while (true) {
        int w = font_.GetWidth(text[i]);
        if (x < clip_b.x_ && x + w < clip_b.x_) {
            x += w;
            if (++i == text.size())
                return;
        } else {
            break;
        }
    }

    text = text.substr(i);
    int left_x = x;
    int y_top = std::max(clip_b.y_, y);
    int y_bottom = std::min(clip_b.y_ + clip_b.h_, y + font_.GetHeight());
    uint8_t mask[MyFont::kMaxFontWidth] {};
    for (char c : text) {
        int right_x = left_x + font_.GetWidth(c);
        int real_left_x = std::max(clip_b.x_, left_x);
        int real_right_x = std::min(clip_b.x_ + clip_b.w_, right_x);
        int mask_x_offset = real_left_x - left_x;
        int mask_len = real_right_x - real_left_x;
        for (int j = y_top; j < y_bottom; ++j) {
            font_.GetMask(c, j - y, mask);
            context_.FillColorHorizenLineMask(j, real_left_x, mask_len, mask + mask_x_offset, color_);
        }

        left_x = right_x;
        if (left_x > clip_b.x_ + clip_b.w_)
            return;
    }
}

void Graphic::DrawEllipse(Bound bound) {
    if (!bound.IsValid())
        return;

    bound.Shifted(component_bound_.x_, component_bound_.y_);
    auto x0 = bound.Left();
    auto x1 = bound.Right();
    auto y0 = bound.Bottom();
    auto y1 = bound.Top();

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
        SetPixel({x1, y0}); /*   I. Quadrant */
        SetPixel({x0, y0}); /*  II. Quadrant */
        SetPixel({x0, y1}); /* III. Quadrant */
        SetPixel({x1, y1}); /*  IV. Quadrant */
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
        SetPixel({x0 - 1, y0}); /* -> finish tip of ellipse */
        SetPixel({x1 + 1, y0++});
        SetPixel({x0 - 1, y1});
        SetPixel({x1 + 1, y1--});
    }
}

void Graphic::FillEllipe(Bound bound) {
    if (!bound.IsValid())
        return;

    bound.Shifted(component_bound_.x_, component_bound_.y_);
    auto center = bound.GetCenter();
    auto top_y = std::max(bound.y_, clip_bound_.y_);
    auto bottom_y = std::min(bound.y_ + bound.h_, clip_bound_.y_ + clip_bound_.h_);
    auto y_it_begin = top_y - center.y_;
    auto y_it_end = bottom_y - center.y_;
    auto a2 = static_cast<float>(bound.w_ * bound.w_ * 0.25f);
    auto b2 = static_cast<float>(bound.h_ * bound.h_ * 0.25f);
    auto div = a2 / b2;
    for (auto y = y_it_begin; y < y_it_end; ++y) {
        auto x2 = a2 - div * y * y;
        auto dx = std::sqrt(x2);
        auto left = static_cast<int>(center.x_ - dx);
        auto right = static_cast<int>(center.x_ + dx);
        left = std::max(left, clip_bound_.x_);
        right = std::min(right, clip_bound_.x_ + clip_bound_.w_);
        context_.FillColorHorizenLine(y + center.y_, left, right - left, color_);
    }
}

void Graphic::MoveDrawContent(Bound aera, int dx, int dy, MyColor background) {
    aera.Shifted(component_bound_.x_, component_bound_.y_);
    aera = aera.GetIntersectionUncheck(clip_bound_);
    if (!aera.IsValid())
        return;

    bool left = dx < 0;
    bool up = dy < 0;
    dx = std::abs(dx);
    dy = std::abs(dy);

    if (dx >= aera.w_ || dy >= aera.h_) {
        context_.FillColorRect(aera, background);
        return;
    }

    if (dx != 0) {
        context_.MoveDrawContentHorizen(aera, dx, left);
        if (left)
            context_.FillColorRect({aera.x_ + aera.w_ - dx, aera.y_, dx, aera.h_}, background);
        else
            context_.FillColorRect({aera.x_, aera.y_, dx, aera.h_}, background);
    }
    if (dy != 0) {
        context_.MoveDrawContentVetical(aera, dy, up);
        if (up)
            context_.FillColorRect({aera.x_, aera.y_ + aera.h_ - dy, aera.w_, dy}, background);
        else
            context_.FillColorRect({aera.x_, aera.y_, aera.w_, dy}, background);
    }
}