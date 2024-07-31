#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <vector>
#include <cstdint>
#include <algorithm>
#include <functional>
#include <cmath>

class TimerTask;

class TimerQueue {
public:
    inline static int min_ms = 10;

    struct OneShotTask {
        std::function<void()> callback;
        int tick_left_{};
    };

    static TimerQueue& GetInstance() {
        static TimerQueue instance;
        return instance;
    }

    void AddTimer(TimerTask* timer) {
        if(std::find(timers_.cbegin(), timers_.cend(), timer) == timers_.cend())
            timers_.push_back(timer);
    }

    void RemoveTimer(TimerTask* timer) {
        timers_.erase(std::remove(timers_.begin(), timers_.end(), timer), timers_.end());
    }

    void AddOneShotTask(std::function<void()> task, float ms) {
        oneshot_tasks_.emplace_back(
            OneShotTask{std::move(task), static_cast<int>(std::round(ms / min_ms))}
        );
    }

    void Tick();
private:
    TimerQueue() {
        timers_.reserve(kInitialTimerCount);
        oneshot_tasks_.reserve(kInitialTimerCount);
    }

    static constexpr auto kInitialTimerCount = 16;
    std::vector<TimerTask*> timers_;
    std::vector<OneShotTask> oneshot_tasks_;
};