#pragma once

#include "adc_panel.h"
#include "osc_panel.h"

class TopWindow : public Component {
public:
    TopWindow() {
        AddChild(&adc_);
        AddChild(&osc_);
    }

    void Resized() override {
        auto b = GetLocalBound();
        adc_.SetBound(b.RemoveFromTop(4 * 8));
        osc_.SetBound(b.RemoveFromTop(4 * 8).Shift(0, 4));
    }
private:
    AdcPanel adc_;
    OscPanel osc_;
};