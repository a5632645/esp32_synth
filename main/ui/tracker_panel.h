#pragma once

#include "../gui/component.h"
#include "st7735_color.h"
#include "../gui/timer_queue.h"

class TrackerComponent : public Component {
public:
    TrackerComponent() {
        TimerQueue::GetInstance().AddTimer(new TimerTask{
            [this]{Repaint(); return false;},
            100,
            -1,
            true
        }, false);
    }

    void DrawSelf(MyGraphic& g) override {
        MyGraphic gg {&frame_};
        auto b = GetLocalBound();
        gg.MoveDrawContent(b, 0, -8, colors::kBlack);
        gg.SetColor(colors::kWhite);
        gg.DrawSingleLineText("tracker", 0, b.Bottom() - 8);

        g.DrawFrame(frame_, b, MyPoint{0, 0});
        g.SetColor(MyColor{rand() & 0xffffffu});
        g.DrawSingleLineText("test", rand() % b.w_, rand() % b.h_);
    }

    void Resized() override {
        auto b = GetLocalBound();
        colors_.resize(b.w_ * b.h_);
        frame_.SetBuffer(colors_.data(), b.w_, b.h_);
    }
private:
    MyColoredFrame<St7735Color> frame_;
    std::vector<typename MyColorTraits<St7735Color>::type> colors_;
};