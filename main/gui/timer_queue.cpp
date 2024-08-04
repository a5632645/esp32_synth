#include "timer_queue.h"

TimerQueue::~TimerQueue() {
    MyScopeLock lock { lock_obj_ };
    for (auto* timer : timers_) {
        timer->queue_ = nullptr;
        if (timer->owned_)
            delete timer;
    }
}

void TimerQueue::AddTimer(TimerTask *timer, bool is_tq_thread, bool start) {
    if (timer == nullptr || !timer->callback_)
        return;

    if (timer->queue_ != nullptr)
        return;

    if (!is_tq_thread) {
        MyScopeLock lock { lock_obj_ };
        timer->queue_ = this;
        timer->paused_ = !start;
        timers_.push_back(timer);
    }
    else {
        timer->queue_ = this;
        timer->paused_ = !start;
        timers_.push_back(timer);
    }
}

void TimerQueue::RemoveTimer(TimerTask *timer, bool is_tq_thread) {
    if (timer == nullptr || timer->queue_ != this)
        return;

    if (!is_tq_thread) {
        MyScopeLock lock { lock_obj_ };
        RemoveTimerUnLock(timer);
    }
    else {
        RemoveTimerUnLock(timer);
    }
}

void TimerQueue::Tick(int ms_eslaped)
{
    auto num_timers = timers_.size();
    for (size_t i = 0; i < num_timers;) {
        auto* timer = timers_[i];

        if (timer->paused_) {
            ++i;
            continue;
        }

        timer->ms_left_ -= ms_eslaped;
        if (timer->ms_left_ > 0) {
            ++i;
            continue;
        }

        while (timer->ms_left_ <= 0)
            timer->ms_left_ += timer->period_ms_;
        if (timer->callback_()) // will call only once even eslaped mutiple period times
            num_timers = timers_.size(); // size changed

        if (timer->repeat_) {
            ++i;
            continue;
        }

        --timer->repeat_count_;
        if (timer->repeat_count_ <= 0) { // need to remove now
            if (timer->owned_) {
                timer->queue_ = nullptr; // set nullptr then destructor will not remove again
                delete timer;
            }
            else {
                timer->queue_ = nullptr;
            }
            std::swap(timers_[i], timers_.back());
            timers_.pop_back();
            --num_timers;
            continue;
        }

        ++i;
    }
}

void TimerQueue::RemoveTimerUnLock(TimerTask *timer) {
    auto it = std::find(timers_.begin(), timers_.end(), timer);
    if (it == timers_.end())
        return;

    timer->queue_ = nullptr;
    std::swap(*it, timers_.back());
    timers_.pop_back();
}

// ================================================================================
TimerTask::~TimerTask() {
    StopAndRemove(false);
}

void TimerTask::Pause(bool is_tq_thread) {
    if (queue_ == nullptr)
        return;

    if (!is_tq_thread) {
        MyScopeLock lock { queue_->lock_obj_ };
        this->paused_ = true;
    }
    else {
        this->paused_ = true;
    }
}

void TimerTask::Resume(bool is_tq_thread) {
    if (queue_ == nullptr)
        return;

    if (!is_tq_thread) {
        MyScopeLock lock { queue_->lock_obj_ };
        this->paused_ = false;
    }
    else {
        this->paused_ = false;
    }
}

void TimerTask::StopAndRemove(bool is_tq_thread) {
    if (queue_ == nullptr) // already deleted
        return;
    queue_->RemoveTimer(this, is_tq_thread);
    queue_ = nullptr;
}
