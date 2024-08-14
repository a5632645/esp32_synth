#pragma once

#include <vector>
#include "my_frame.h"
#include "bound.h"
#include "my_font.h"

enum class MyTextAlign {
    kXLeft,
    kXCenter,
    kXRigh,
    kYUp,
    kYDown,
    kYCenter
};

class IMyRender {
public:
    virtual ~IMyRender() = default;
    IMyRender(MyFrame& f) : frame_(f) {}
    void SetClipBound(const Bound& bound) {
        clip_bound_ = bound;
    }

    virtual void DrawPixel(MyPoint p, MyColor c) = 0;

    struct LineReq {
        MyPoint start;
        MyPoint end;
        MyColor c;
    };
    virtual void DrawLine(const LineReq& req) = 0;

    struct RectReq {
        Bound aera;
        MyColor c;
    };
    virtual void FillRect(const RectReq& req) = 0;
    virtual void DrawRect(const RectReq& req) = 0;

    struct TextReq {
        MyFont& font;
        std::string_view t;
        MyPoint pos;
        MyColor c;
    };
    virtual void DrawText(const TextReq& req) = 0;

    struct AlignTextReq {
        MyFont& font;
        std::string_view t;
        Bound aera;
        MyColor c;
    };
    virtual void DrawTextAlign(const AlignTextReq& req) = 0;

    struct PathReq {
        MyPoint* pdata;
        size_t len;
        MyColor c;
    };
    virtual void DrawPath(const PathReq& req) = 0;
    virtual void FillPath(const PathReq& req) = 0;

    struct FrameReq {
        MyFrame& frame;
    };
    virtual void DrawFrame(const FrameReq& req) = 0;

    struct EllipseReq {
        Bound aera;
        MyColor c;
    };
    virtual void DrawEllipse(const EllipseReq& req) = 0;
    virtual void FillEllipse(const EllipseReq& req) = 0;

    struct MoveContentReq {
        Bound aera;
        int dx;
        int dy;
    };
    virtual void MoveDrawContent(const MoveContentReq& req) = 0;
protected:
    Bound clip_bound_;
    MyFrame& frame_;
};
