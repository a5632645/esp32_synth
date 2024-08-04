#include "keyboard_panel.h"
#include "table_panel.h"
#include "my_events.h"

KeyboardPanel::KeyboardPanel() {
    for (auto i : kWhiteKeyIndex) {
        keys_[i].color_ = colors::kWhite;
        keys_[i].off_color_ = colors::kWhite;
        AddChild(&keys_[i]);
    }
    for (auto i : kBlackKeyIndex) {
        keys_[i].color_ = colors::kBlack;
        keys_[i].off_color_ = colors::kBlack;
        AddChild(&keys_[i]);
    }
}

void KeyboardPanel::Resized() {
    auto b = GetLocalBound();
    b.RemoveFromTop(8);
    
    auto w = b.w_ / 7;
    for (int i = 0; i < 7; ++i) {
        auto bb = b.RemoveFromLeft(w);
        --bb.w_;
        keys_[kWhiteKeyIndex[i]].SetBound(bb);
    }

    b = GetLocalBound();
    b.RemoveFromTop(8);
    w = b.w_ / 14;
    auto xshift = w / 2;
    auto h = b.h_ / 2;
    keys_[1].SetBound(Bound{w + xshift, 8, w - 1, h});
    keys_[3].SetBound(Bound{w * 3 + xshift, 8, w - 1, h});
    keys_[6].SetBound(Bound{w * 7 + xshift, 8, w - 1, h});
    keys_[8].SetBound(Bound{w * 9 + xshift, 8, w - 1, h});
    keys_[10].SetBound(Bound{w * 11 + xshift, 8, w - 1, h});
}

void KeyboardPanel::OnEventGet(const MyEvent& e) {
    if (flags_.focus_ != FocusState::kFoucusSelf || e.event_type != events::kButtonDown)
        return;

    switch (e.sub_type) {
    case events::kExitComp:
        ExitFocus();
        GetParent()->Repaint();
        break;
    case events::kButton14:
        if (global_model.curr_octave == 9)
            break;
        ++global_model.curr_octave;
        Repaint(GetLocalBound().WithHeight(8));
        break;
    case events::kButton16:
        if (global_model.curr_octave == -1)
            break;
        --global_model.curr_octave;
        Repaint(GetLocalBound().WithHeight(8));
    default:
        break;
    }
}
