#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include "../gui/my_driver.h"
#include "st7735_color.h"
#include "ili9340.h"

class LcdDriver2 : public MyDriver {
public:
    static constexpr auto kWidth = 240;
    static constexpr auto kHeight = 320;

    void Init();

    void BeginFrame() override {}
    void AeraDrawed(const Bound& bound) override {}
    void EndFrame(const Bound& bound) override;

    MyFrame* GetFrame() override {
        return &frame_;
    }

    TFT_t dev_;
    MyColoredFrame<St7735Color> frame_;
};