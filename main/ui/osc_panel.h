#pragma once

#include "gui/component.h"
#include "gui/timer_task.h"
#include "gui/msg_queue.h"

class OscPanel : public Component {
public:
    void PaintSelf(Graphic& g) override {
        auto cb = g.GetClipBound();
        auto b = GetLocalBound();
        auto center_y = b.GetCenter().y_;
        auto h = b.h_ / 2;
        g.Fill(colors::kBlack);
        g.SetColor(colors::kGreen);
        for (int i = cb.x_; i < cb.x_ + cb.w_; ++ i) {
            auto sample = samples_[i];
            auto top = static_cast<int>(center_y - sample * h);
            auto bottom = center_y;
            if (sample < 0.0f)
                std::swap(top, bottom);
            g.DrawVeticalLine(i, top, bottom - top);
        }

        if (cb == b)
            finishing_ = true;
    }

    void PushSample(float* sample, int len) {
        if (!finishing_)
            return;

        for (int i = 0; i < len; i += 8) {
            samples_[current_index_++] = sample[i];
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
    std::vector<float> samples_;
    bool finishing_ = false;
    int current_index_ = 0;
};