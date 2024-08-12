#pragma once

#include "adc_panel.h"
#include "osc_panel.h"
#include "table_panel.h"

class TopWindow : public Component {
public:
    TopWindow() {
        AddChild(&adc_);
        AddChild(&osc_);
        AddChild(&table_);

        table_.GetFocus();
    }

    void DrawSelf(MyGraphic& g) override {
        g.Fill(colors::kBlack);
    }

    void Resized() override {
        auto b = GetLocalBound();
        adc_.SetBound(b.RemoveFromTop(4 * 8));
        osc_.SetBound(b.RemoveFromTop(4 * 8));
        table_.SetBound(b);
    }

    void OnEventGet(const MyEvent& e) override {
        table_.OnEventGet(e);
    }

    KeyboardPanel& GetKeyboardPanel() { return table_.GetKeyboardPanel(); }
private:
    AdcPanel adc_;
    OscPanel osc_;
    TablePanel table_;
};