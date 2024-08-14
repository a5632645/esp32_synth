#pragma once

#include "gui/component.h"
#include "gui/msg_queue.h"

class OscPanel : public Component {
public:
    OscPanel() {
        task_ = TimerTask {
            [this]{ finishing_ = true; return false; },
            100
        };
        TimerQueue::GetInstance().AddTimer(&task_, false, true);
    }

    void DrawSelf(MyGraphic& g) override {
        auto cb = g.GetClipBound();
        auto b = GetLocalBound();
        auto center_y = b.GetCenter().y_;
        auto h = b.h_ / 2;
        g.SetColor(colors::kBlue);
        for (int i = cb.x_; i < cb.x_ + cb.w_; ++ i) {
            auto sample = samples_[i];
            auto top = static_cast<int>(center_y - sample * h);
            auto bottom = center_y;
            if (sample < 0.0f)
                std::swap(top, bottom);
            g.DrawVeticalLine(i, top, bottom - top);
        }
        g.DrawRect(b);
    }

    void PushSample(int16_t* sample, int len) {
        if (samples_.size() == 0)
            return;
        if (!finishing_)
            return;

        constexpr auto inv_max = 1.0f / 0x2000;
        for (int i = 0; i < len; i += 8) {
            samples_[current_index_++] = sample[i] * inv_max;
            if (current_index_ >= samples_.size()) {
                current_index_ = 0;
                MsgQueue::GetInstance().Push({
                    .handler = [this, i = current_index_]() {
                        Repaint();
                    }
                });
                return;
            }
        }
    }

    void Resized() {
        finishing_ = false;
        samples_.resize(GetLocalBound().w_);
        finishing_ = true;
    }
private:
    TimerTask task_;
    std::vector<float> samples_;
    bool finishing_ = false;
    int current_index_ = 0;
};