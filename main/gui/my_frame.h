#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include "color.h"
#include "bound.h"

// buffer are always y * w + x or draw command(?)
class MyFrame {
public:
    MyFrame() : buffer_(nullptr), w_(0), h_(0) {}
    explicit MyFrame(void* buffer, int w, int h) : buffer_(buffer), w_(w), h_(h) {}
    virtual ~MyFrame() = default;

    void SetBuffer(void* buffer, int w, int h) {
        buffer_ = buffer;
        w_ = w;
        h_ = h;
    }

    virtual void SetColor(MyColor c) = 0;
    virtual void DrawPoint(int x, int y) = 0;
    virtual void DrawPointColor(int x, int y, MyColor c) = 0;
    virtual void DrawHorizenLine(int y, int x, int w) = 0;
    virtual void DrawVeticalLine(int x, int y, int h) = 0;
    virtual void FillRect(const Bound& bound, MyColor c) = 0;
    virtual void DrawHorizenLineMask(int y, int x, int w, uint8_t* alpha_mask) = 0;

    virtual void MoveDrawContentHorizen(const Bound& aera, int offset, bool left) = 0;
    virtual void MoveDrawContentVetical(const Bound& aera, int offset, bool up) = 0;

    Bound GetBound() const {
        return Bound{ 0, 0, w_, h_ };
    }
protected:
    void* buffer_{};
    int w_ {};
    int h_ {};
};

// some color type
using MyRGB888 = struct { using type = uint32_t; };
using MyRGB565 = struct { using type = uint16_t; };
using MyBGR565 = struct { using type = uint16_t; };
using MyRGB332 = struct { using type = uint8_t; };
using MyMono = struct { using type = uint8_t; };

// a color traits
template<typename T>
struct MyColorTraits {
    using type = typename T::type;

    static constexpr type ColorTransform(MyColor c) {
        return {};
    }
};

// implement frame for some color type
template<class T>
class MyColoredFrame : public MyFrame {
public:
    using ColorType = typename MyColorTraits<T>::type;
    using MyFrame::MyFrame;

    void SetColor(MyColor c) override;
    void DrawPoint(int x, int y) override;
    void DrawPointColor(int x, int y, MyColor c) override;
    void DrawHorizenLine(int y, int x, int w) override;
    void DrawVeticalLine(int x, int y, int h) override;
    void FillRect(const Bound& bound, MyColor c) override;
    void DrawHorizenLineMask(int y, int x, int w, uint8_t* alpha_mask) override;

    void MoveDrawContentHorizen(const Bound& aera, int offset, bool left) override;
    void MoveDrawContentVetical(const Bound& aera, int offset, bool up) override;

    ColorType* GetPtr(int x, int y) {
        return static_cast<ColorType*>(buffer_) + y * w_ + x;
    }
private:
    ColorType color_ {};
};

template<>
struct MyColorTraits<MyRGB888> {
    using type = uint32_t;
    static constexpr type ColorTransform(MyColor c) {
        return ((c.r & 0xFF) << 16) | ((c.g & 0xFF) << 8) | (c.b & 0xFF);
    }
};

template<>
struct MyColorTraits<MyRGB565> {
    using type = uint16_t;
    static constexpr type ColorTransform(MyColor c) {
        return ((c.r & 0xF8) << 8) | ((c.g & 0xFC) << 3) | (c.b >> 3);
    }
};

template<>
struct MyColorTraits<MyBGR565> {
    using type = uint16_t;
    static constexpr type ColorTransform(MyColor c) {
        return ((c.b & 0xF8) << 8) | ((c.g & 0xFC) << 3) | (c.r >> 3);
    }
};
