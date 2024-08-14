#include "../my_render.h"
#include "my_color_trait.h"

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
    void DrawTextAlign(const AlignTextReq& req) override;
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
    if (clip_bound_.ContainPoint(p.x_, p.y_))
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

}

template <typename ColorTrait>
void MyRender<ColorTrait>::DrawVeticalline(const LineReq& req) {
    auto x = req.start.x_;
    auto y = std::min(req.start.y_, req.end.y_);
    auto y_end = std::max(req.start.y_, req.end.y_);
    auto c = ColorTransform(req.c);
    for (; y <= y_end; ++y) {
        *GetXYPtr(x, y) = c;
    }
}

template <typename ColorTrait>
void MyRender<ColorTrait>::DrawHorizenLine(const LineReq& req) {
    auto y = req.start.y_;
    auto x = std::min(req.start.x_, req.end.x_);
    auto len = std::abs(req.start.x_ - req.end.x_);
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
void MyRender<ColorTrait>::DrawText(const TextReq&req) {
}

template <typename ColorTrait>
void MyRender<ColorTrait>::DrawTextAlign(const AlignTextReq& req) {
}

template <typename ColorTrait>
void MyRender<ColorTrait>::DrawPath(const PathReq& req) {
    if (req.size < 2)
        return;

    LineReq lreq;
    lreq.start = req.points[0];
    lreq.c = req.c;
    for (size_t i = 1;
         i < req.len;
         ++i) {
        lreq.end = req.pdata[i];
        this->DrawLine(lreq);
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
}

template <typename ColorTrait>
void MyRender<ColorTrait>::FillEllipse(const EllipseReq& req) {
}

template <typename ColorTrait>
void MyRender<ColorTrait>::MoveDrawContent(const MoveContentReq& req) {
}
