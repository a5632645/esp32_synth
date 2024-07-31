#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include "lcd.h"
#include "gui/ll_context.h"

class St7735LLContext : public LLContext {
public:
    static constexpr auto kWidth = 128;
    static constexpr auto kHeight = 160;

    void BatchSetColor(int* x, int* y, MyColor c, int len) override;
    void SetColor(int x, int y, MyColor c) override;
    void FillColorHorizenLine(int y, int x, int w, MyColor c) override;
    void FillColorVeticalLine(int x, int y, int h, MyColor c) override;
    void FillColorRect(Bound aera, MyColor c) override;
    void FillColorHorizenLineMask(int y, int x, int w, uint8_t* mask, MyColor c) override;

    Bound GetBound() const override {
        return Bound{ 0, 0, kWidth, kHeight };
    }

    void MoveDrawContentHorizen(Bound aera, int offset, bool left) override;
    void MoveDrawContentVetical(Bound aera, int offset, bool up) override;

    void EndFrame(Bound dirty_aera) override;

    ST7735_t dev;
    alignas(32) uint16_t screen_buffer[kWidth * kHeight] {};
};