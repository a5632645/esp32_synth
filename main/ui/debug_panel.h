#pragma once

#include "../gui/component.h"
#include "../gui/timer_queue.h"

class DebugPanel : public Component {
public:
    DebugPanel();

    void DrawSelf(MyGraphic& g) override;
    void OnGetFocus() override;

    void Resume() { timer_task_.Resume(false); }
    void Pause() { timer_task_.Pause(false); }
private:
    TimerTask timer_task_;
};