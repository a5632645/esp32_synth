#pragma once

#include "gui/component.h"
#include <string>

class AdcPanel : public Component {
public:
    void SetAdcVal(int index, int val) {
        adc_vals_[index] = val;
        auto b = GetLocalBound();
        b.y_ = index * 8;
        b.h_ = 8;
        Repaint(b);
    }

    void PaintSelf(Graphic& g) override {
        g.Fill(colors::kBlack);
        g.SetColor(colors::kWhite);
        for (int i = 0; i < 4; i++) {
            g.DrawSingleLineText("ADC" + std::to_string(i) + ":" + std::to_string(adc_vals_[i]), 0, i * 8, -1);
        }
    }
private:
    int adc_vals_[4] {};
};