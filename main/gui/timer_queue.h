#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <vector>
#include <cstdint>
#include <algorithm>
#include <functional>
#include "gui_config.h"

class TimerQueue;

class TimerTask {
public:
    TimerTask() = default;

    /**
     * Constructs a TimerTask object with the given callback, period, repeat count, and ownership flag.
     *
     * @param callback The function to be called when the timer expires, return true if you add timertask or remove task in a timertask
     * @param period_ms The period in milliseconds at which the timer should expire.
     * @param repeat_count The number of times the timer should repeat. If negative, the timer will repeat indefinitely. Default is -1.
     * @param owned Flag indicating whether the TimerTask object should take ownership of the object. Default is false.
     * @note  if not owned, please check the object not change memory address!
     */
    TimerTask(std::function<bool()> callback,
              int period_ms,
              int repeat_count = -1,
              bool owned = false)
        : callback_(std::move(callback)),
          queue_(nullptr),
          period_ms_(period_ms),
          ms_left_(period_ms),
          repeat_count_(repeat_count),
          owned_(owned),
          repeat_(repeat_count < 0) {}

    ~TimerTask();
    TimerTask(const TimerTask&) = delete;
    TimerTask& operator=(const TimerTask&) = delete;
    TimerTask(TimerTask&&) = default;
    TimerTask& operator=(TimerTask&&) = default;

    void Start(bool is_tq_thread) { Resume(is_tq_thread); }
    void Pause(bool is_tq_thread);
    void Resume(bool is_tq_thread);
    void StopAndRemove(bool is_tq_thread);
private:
    friend class TimerQueue;

    std::function<bool()> callback_;
    TimerQueue* queue_ = nullptr;
    int period_ms_ = 0;
    int ms_left_ = 0;
    int repeat_count_ = 0;
    bool owned_ = false;
    bool repeat_ = false;
    bool paused_ = true;
};

class TimerQueue {
public:
    static TimerQueue& GetInstance() {
        static TimerQueue instance;
        return instance;
    }

    TimerQueue() {
        timers_.reserve(16);
    }
    ~TimerQueue();

    void AddTimer(TimerTask* timer, bool is_tq_thread, bool start = true);
    void RemoveTimer(TimerTask* timer, bool is_tq_thread);
    void Tick(int ms_eslaped);
private:
    friend class TimerTask;

    void RemoveTimerUnLock(TimerTask* timer);

    MyLock lock_obj_;
    std::vector<TimerTask*> timers_;
};