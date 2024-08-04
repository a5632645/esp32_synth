#pragma once

#include "gui/component.h"
#include "gui/timer_queue.h"
#include "model/synth_model.h"
#include <string>

class AdcPanel : public Component {
public:
    AdcPanel() {
        timer_task_ = TimerTask{
            [this]{ Repaint(); return false; },
            100
        };
        TimerQueue::GetInstance().AddTimer(&timer_task_, false, true);
    }

    void DrawSelf(Graphic& g) override {
        g.Fill(colors::kBlack);
        g.SetColor(colors::kWhite);
        for (int i = 0; i < 4; i++) {
            g.DrawSingleLineText("ADC" + std::to_string(i) + ":" + std::to_string(global_model.adc_vals[i]), 0, i * 8, -1);
        }
    }
private:
    TimerTask timer_task_;
};