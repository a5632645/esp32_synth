#pragma once

#include <vector>
#include "my_frame.h"
#include "bound.h"
#include "my_font.h"

typedef enum {
    kXLeft = 0b1,
    kXCenter = 0b10,
    kXRigh = 0b100,
    kYUp = 0b1000,
    kYDown = 0b10000,
    kYCenter = 0b100000
} MyTextAlign;
typedef int MyTextAlignFlags;

class IMyRender {
public:
    virtual ~IMyRender() = default;
    IMyRender(MyFrame& f) : frame_(f) {}

    virtual void DrawPixel(MyPoint p, MyColor c) = 0;

    struct LineReq {
        Bound clip;
        MyColor c;
        MyPoint end;
        MyPoint start;
    };
    virtual void DrawLine(const LineReq& req) = 0;

    struct RectReq {
        Bound aera;
        Bound clip;
        MyColor c;
    };
    virtual void FillRect(const RectReq& req) = 0;
    virtual void DrawRect(const RectReq& req) = 0;

    struct TextReq {
        Bound aera;
        MyColor c;
        MyFont& font;
        std::string_view t;
        MyPoint topleft;
    };
    virtual void DrawText(const TextReq& req) = 0;

    struct PathReq {
        MyColor c;
        Bound clip;
        size_t len;
        MyPoint* pdata;
    };
    virtual void DrawPath(const PathReq& req) = 0;
    virtual void FillPath(const PathReq& req) = 0;

    struct FrameReq {
        MyFrame& frame;
    };
    virtual void DrawFrame(const FrameReq& req) = 0;

    struct EllipseReq {
        Bound aera;
        Bound clip;
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
    MyFrame& frame_;
};
