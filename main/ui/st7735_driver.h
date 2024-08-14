#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include "lcd.h"
#include "../gui/my_driver.h"
#include "st7735_color.h"

class St7735Driver : public MyDriver {
public:
    static constexpr auto kWidth = 128;
    static constexpr auto kHeight = 160;

    void Init();

    void BeginFrame() override;
    void AeraDrawed(const Bound& bound) override;
    void EndFrame(const Bound& bound) override;

    MyFrame* GetFrame() override {
        return &frame_;
    }

    ST7735_t dev_;
    MyColoredFrame<St7735Color> frame_;
};