#include "debug_panel.h"

DebugPanel::DebugPanel() {
    timer_task_ = TimerTask{
        [this]{ Repaint(); return false; },
        500
    };
    TimerQueue::GetInstance().AddTimer(&timer_task_, false, false);
}

void DebugPanel::DrawSelf(MyGraphic& g) {
    g.Fill(colors::kBlack);

    auto b = GetLocalBound();
    if (!b.IsValid())
        return;

    auto w = rand() % b.w_;
    auto h = rand() % b.h_;
    uint32_t rd_color = static_cast<uint32_t>(rand() % 0xffffff);

    g.SetColor(MyColor{rd_color});
    g.DrawSingleLineText("debug", w, h);
}

void DebugPanel::OnGetFocus() {
    ExitFocus();
    GetParent()->Repaint();
}
