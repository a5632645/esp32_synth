#include "table_panel.h"

#include "my_events.h"

TablePanel::TablePanel() {
    AddChild(&tab_bar_);
    AddChild(&setting_panel_);
    AddChild(&keyboard_);

    InternalSwitchComponent(1);
}

void TablePanel::OnEventGet(const MyEvent& e) {
    switch (flags_.focus_) {
    case FocusState::kFoucusChild:
        for (auto& c : children_)
            c->OnEventGet(e);
        break;
    case FocusState::kFoucusSelf:
        HandleEvent(e);
        break;
    default:
        break;
    }
}

void TablePanel::DrawAboveChild(MyGraphic& g) {
    if (flags_.focus_ != FocusState::kFoucusSelf)
        return;

    g.SetColor(colors::kOrange);
    g.DrawRect(GetChildUncheck(comp_select_index)->GetBoundInParent());
}

void TablePanel::Resized() {
    auto b = GetLocalBound();
    tab_bar_.SetBound(b.RemoveFromTop(8));
    keyboard_.SetBound(b.RemoveFromBottom(24));
    if (auto* curr = GetChildUncheck(1); curr != nullptr)
        curr->SetBound(b);
}

void TablePanel::InternalSwitchComponent(uint16_t index) {
    switch (index) {
    case 0:
        ReplaceChild(1, &setting_panel_);
        break;
    case 1:
        ReplaceChild(1, &tracker_panel_);
        break;
    case 2:
        ReplaceChild(1, &debug_panel_);
        break;
    default:
        break;
    }

    if (index == 2)
        debug_panel_.Resume();
    else
        debug_panel_.Pause();
}

void TablePanel::HandleEvent(const MyEvent& e) {
    if (e.event_type != events::kButtonDown)
        return;

    switch (e.sub_type) {
    case events::kEnterComp:
        if (comp_select_index == 0) {
            tab_bar_.GetFocus();
        }
        else if (comp_select_index == 1) {
            GetChildUncheck(1)->GetFocus();
        }
        else {
            keyboard_.GetFocus();
        }
        Repaint();
        break;
    case events::kButton14: // prev child
        if (comp_select_index == 0)
            break;
        --comp_select_index;
        Repaint();
        break;
    case events::kButton16: // next child
        if (comp_select_index == 2)
            break;
        ++comp_select_index;
        Repaint();
        break;
    default:
        break;
    }
}

// ================================================================================
void TablePanel::TableTabBar::OnEventGet(const MyEvent& e) {
    if (flags_.focus_ != FocusState::kFoucusSelf || e.event_type != events::kButtonDown)
        return;

    switch (e.sub_type) {
    case events::kExitComp: // exit tab bar
        ExitFocus();
        GetParent()->Repaint();
        break;
    case events::kButton14: // prev option
        if (curr_select_index == 0)
            break;

        --curr_select_index;
        static_cast<TablePanel*>(GetParent())->InternalSwitchComponent(curr_select_index);
        Repaint();
        break;
    case events::kButton16: // next option
        if (curr_select_index == static_cast<uint16_t>(titles_.size()) - 1)
            break;

        ++curr_select_index;
        static_cast<TablePanel*>(GetParent())->InternalSwitchComponent(curr_select_index);
        Repaint();
        break;
    default:
        break; 
    }
}

void TablePanel::TableTabBar::DrawSelf(MyGraphic& g) {
    auto& font = g.GetFont();
    g.Fill(colors::kBlack);

    g.SetColor(colors::kWhite);
    int draw_x = 0;
    if (curr_select_index > 0) { // draw prev option
        g.DrawSingleLineText(titles_[curr_select_index - 1], draw_x, 0, -1);
        draw_x += font.GetWidth(titles_[curr_select_index - 1]);
        ++draw_x;
    }

    // give a high light
    auto w = font.GetWidth(titles_[curr_select_index]);
    if (flags_.focus_ == FocusState::kFoucusSelf) {
        g.FillRect(Bound(draw_x, 0, w, 8), colors::kOrange);
        g.SetColor(colors::kWhite);
        g.DrawSingleLineText(titles_[curr_select_index], draw_x, 0, -1);
    }
    else {
        g.SetColor(colors::kOrange);
        g.DrawSingleLineText(titles_[curr_select_index], draw_x, 0, -1);
    }
    draw_x += (w + 1);

    g.SetColor(colors::kWhite);
    uint16_t size = static_cast<uint16_t>(titles_.size());
    for (uint16_t i = curr_select_index + 1; i < size ; ++i) { // draw else option
        if (draw_x >= g.GetClipBound().Right())
            return;

        g.DrawSingleLineText(titles_[i], draw_x, 0, -1);
        draw_x += font.GetWidth(titles_[i]);
        ++draw_x;
    }
}