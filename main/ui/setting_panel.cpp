#include "setting_panel.h"

#include "my_events.h"

SettingPanel::SettingPanel() {
    using namespace std::string_view_literals;

    comps_.reserve(4);
    comps_.emplace_back(&test_, "test"sv, std::vector{"a"sv, "b"sv, "c"sv});
    comps_.emplace_back(&test_, "test2"sv, std::vector{"d"sv, "e"sv, "f"sv, "g"sv});
    comps_.emplace_back(&test_, "test3"sv, std::vector{"h"sv, "i"sv, "j"sv, "k"sv});
    comps_.emplace_back(&test_, "test4"sv, std::vector{"l"sv, "m"sv, "n"sv, "o"sv});

    for (auto& comp : comps_)
        AddChild(&comp);
}

void SettingPanel::OnEventGet(const MyEvent &e) {
    switch (flags_.focus_) {
    case FocusState::kFoucusChild:
        comps_[curr_setting_index].OnEventGet(e);
        break;
    case FocusState::kFoucusSelf:
        HandleEvent(e);
        break;
    default:
        break;
    }
}

void SettingPanel::Resized() {
    auto b = GetLocalBound().WithHeight(8);
    for (auto& comp : comps_) {
        comp.SetBound(b);
        b.Shifted(0, 8);
    }
}

void SettingPanel::DrawSelf(Graphic& g) {
    g.Fill(colors::kBlack);

    if (flags_.focus_ == FocusState::kFoucusSelf) {
        Bound b{0, curr_setting_index * 8, GetLocalBound().w_, 8};
        g.SetColor(colors::kWhite);
        g.DrawRect(b);
    }
}

void SettingPanel::HandleEvent(const MyEvent& e) {
    if (e.event_type != events::kButtonDown)
        return;

    switch (e.sub_type) {
    case events::kButton14: // up
        if (curr_setting_index == 0)
            break;
        --curr_setting_index;
        Repaint();
        break;
    case events::kButton16: // down
        if (curr_setting_index == static_cast<uint16_t>(comps_.size() - 1))
            break;
        ++curr_setting_index;
        Repaint();
        break;
    case events::kEnterComp:
        ChildGetFocus(comps_.data() + curr_setting_index);
        Repaint();
        break;
    case events::kExitComp:
        ExitFocus();
        GetParent()->Repaint();
        break;
    default:
        break;
    }
}

// ================================================================================
// OptionComponent
// ================================================================================
void SettingPanel::OptionComponent::DrawSelf(Graphic& g) {
    if (flags_.focus_ == FocusState::kFoucusSelf)
        g.Fill(colors::kOrange);

    g.SetColor(colors::kWhite);
    g.DrawSingleLineText(title_, 0, 0);
    g.DrawSingleLineText(options_[*value_], 0, 0, GetLocalBound().w_, MyJustification::kRight);
}

void SettingPanel::OptionComponent::OnEventGet(const MyEvent& e) {
    if (flags_.focus_ != FocusState::kFoucusSelf || e.event_type != events::kButtonDown)
        return;

    switch (e.sub_type) {
    case events::kExitComp:
        ExitFocus();
        GetParent()->Repaint();
        break;
    case events::kButton14: // up
        if (*value_ == 0)
            break;

        --(*value_);
        Repaint();
        break;
    case events::kButton16: // down
        if (*value_ == static_cast<uint16_t>(options_.size() - 1))
            break;

        ++(*value_);
        Repaint();
        break;
    default:
        break;
    }
}
