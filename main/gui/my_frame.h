#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include "color.h"
#include "bound.h"

// buffer are always y * w + x
class MyFrame {
public:
    virtual ~MyFrame() = default;

    virtual void SetColor(MyColor c) = 0;
    virtual void DrawPoint(int16_t x, int16_t y) = 0;
    virtual void DrawPointColor(int16_t x, int16_t y, MyColor c) = 0;
    virtual void DrawHorizenLine(int16_t y, int16_t x, int16_t w) = 0;
    virtual void DrawVeticalLine(int16_t x, int16_t y, int16_t h) = 0;
    virtual void FillRect(const Bound& bound, MyColor c) = 0;
    virtual void DrawHorizenLineMask(int16_t y, int16_t x, int16_t w, uint8_t* alpha_mask) = 0;

    virtual void MoveDrawContentHorizen(const Bound& aera, int16_t offset, bool left) = 0;
    virtual void MoveDrawContentVetical(const Bound& aera, int16_t offset, bool up) = 0;

    Bound GetBound() const {
        return Bound{ 0, 0, w_, h_ };
    }
protected:
    int16_t w_ {};
    int16_t h_ {};
};

namespace frame_colors {
using RGB888 = uint32_t;
using RGB565 = uint16_t;
using BGR565 = uint16_t;
using RGB332 = uint8_t;
using Mono = uint8_t; 

template<typename T>
inline static constexpr T ColorTransform(MyColor c) {
    if constexpr (std::is_same_v<T, RGB888>) {
        return static_cast<T>(((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF));
    }
    else if constexpr (std::is_same_v<T, RGB565>) {
        return static_cast<T>(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
    }
    else if constexpr (std::is_same_v<T, BGR565>) {
        return static_cast<T>(((b & 0xF8) << 8) | ((g & 0xFC) << 3) | (r >> 3));
    }
    else if constexpr (std::is_same_v<T, RGB332>) {
        return static_cast<T>((r >> 5) | ((g >> 5) << 5) | ((b >> 6) << 10));
    }
    else if constexpr (std::is_same_v<T, Mono>) {
        return static_cast<T>((r | g | b) == 0 ? 1 : 0);
    }
    else {
        static_assert(false, "unknown color type");
    }
}
}

template<class T>
class MyColoredFrame : public MyFrame {
public:
    void SetColor(MyColor c) override { color_ = frame_colors::ColorTransform<T>(c); }
    void DrawPoint(int16_t x, int16_t y) override { *GetPtr(x, y) = color_; }
    void DrawPointColor(int16_t x, int16_t y, MyColor c) override { *GetPtr(x, y) = frame_colors::ColorTransform<T>(c); }
    void DrawHorizenLine(int16_t y, int16_t x, int16_t w) override;
    void DrawVeticalLine(int16_t x, int16_t y, int16_t h) override;
    void FillRect(const Bound& bound, MyColor c) override;
    void DrawHorizenLineMask(int16_t y, int16_t x, int16_t w, uint8_t* alpha_mask) override;

    void MoveDrawContentHorizen(const Bound& aera, int16_t offset, bool left) override;
    void MoveDrawContentVetical(const Bound& aera, int16_t offset, bool up) override;

    T* GetPtr(int16_t x, int16_t y) {
        return buffer_ + y * w_ + x;
    }
private:
    T color_   {};
    T* buffer_ {};
};

template<>
class MyColoredFrame<frame_colors::Mono> : public MyFrame {
};

