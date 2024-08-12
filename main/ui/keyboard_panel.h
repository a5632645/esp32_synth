#pragma once

#include <string>
#include "gui/component.h"
#include "gui/msg_queue.h"
#include "model/synth_model.h"

class KeyboardPanel : public Component {
public:
    static constexpr int kWhiteKeyIndex[] {
        0, 2, 4, 5, 7, 9, 11
    };
    static constexpr int kBlackKeyIndex[] {
        1, 3, 6, 8, 10
    };

    KeyboardPanel();
    void Resized() override;

    void NoteOn(int note) {
        keys_[note % 12].NoteOn();
    }

    void NoteOff(int note) {
        keys_[note % 12].NoteOff();
    }

    void DrawSelf(MyGraphic& g) override {
        if (flags_.focus_ == FocusState::kFoucusSelf) {
            g.FillRect(GetLocalBound(), colors::kOrange);
        }

        g.SetColor(colors::kWhite);
        g.DrawSingleLineText("octave: " + std::to_string(global_model.curr_octave), 0, 0, -1);
    }

    void OnEventGet(const MyEvent& e) override;
private:
    struct KeyComponent : public Component {
        void DrawSelf(MyGraphic& g) override {
            g.Fill(color_);
        }

        void NoteOn() {
            color_ = colors::kGrey;
            MsgQueue::GetInstance().Push(
                {.handler = [this]() { Repaint(); } }
            );
        }

        void NoteOff() {
            color_ = off_color_;
            MsgQueue::GetInstance().Push(
                {.handler = [this]() { Repaint(); } }
            );
        }

        MyColor color_;
        MyColor off_color_;
    } keys_[12] {};
};