#include "timer_queue.h"
#include "timer_task.h"

void TimerQueue::Tick() {
    for(auto& timer : timers_) {
        timer->tick_left_ -= 1;
        if(timer->tick_left_ == 0) {
            timer->TimerCallback();
            timer->tick_left_ = timer->total_tick_;
        }
    }
    for(auto it = oneshot_tasks_.begin(); it != oneshot_tasks_.end(); /*no increment*/) {
        auto& task = *it;
        task.tick_left_ -= 1;
        if(task.tick_left_ == 0) {
            if(task.callback) {
                task.callback();
            }
            it = oneshot_tasks_.erase(it);
        }
        else {
            ++it;
        }
    }
}