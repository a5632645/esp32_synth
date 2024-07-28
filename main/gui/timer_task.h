#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <cmath>
#include "timer_queue.h"

class TimerTask {
public:
    virtual ~TimerTask() {
        StopTimer();
    }
    virtual void TimerCallback() = 0;

    void StartTimerMs(float ms) {
        total_tick_ = static_cast<int>(std::round(ms / TimerQueue::min_ms));
        tick_left_ = total_tick_;
        TimerQueue::GetInstance().AddTimer(this);
    }

    void StartTimerHz(float hz) {
        StartTimerMs(1000.0f / hz);
    }

    void StopTimer() {
        TimerQueue::GetInstance().RemoveTimer(this);
    }
private:
    friend class TimerQueue;

    int total_tick_{};
    int tick_left_{};
};