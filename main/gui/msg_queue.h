#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <array>
#include <cstdint>
#include <functional>

namespace cmds {
static constexpr auto kPaint = 1;
}

class MsgQueue {
private:
    inline static bool has_message = false;
    inline static bool lock = false;

    static void DummyWait(void*) {
        while (!has_message) {}
    }

    static void DummyLock(void*) {
        while (lock) {}
        lock = true;
    }

    static void DummyUnLock(void*) {
        lock = false;
    }

    static void DummyNotify(void*) {
        has_message = true;
    }

public:
    inline static void* msg_notify_obj = nullptr;
    inline static void* lock_obj = nullptr;
    inline static void(*wait)(void*) = DummyWait;
    inline static void(*notify)(void*) = DummyNotify;
    inline static void(*get_lock)(void*) = DummyLock;
    inline static void(*release_lock)(void*) = DummyUnLock;

    struct Message {
        int command = 0;
        std::function<void()> handler;
    };

    static MsgQueue& GetInstance() {
        static MsgQueue instance;
        return instance;
    }

    bool Push(Message message) {
        struct ScopeLock {
            ScopeLock() {
                MsgQueue::get_lock(MsgQueue::lock_obj);
            }
            ~ScopeLock() {
                MsgQueue::release_lock(MsgQueue::lock_obj);
            }
        } s;

        if(count_ == kMaxMessageCount) {
            return false;
        }

        messages_[write_pos_] = std::move(message);
        write_pos_ = (write_pos_ + 1) & (kMaxMessageCount - 1);
        ++count_;
        MsgQueue::notify(MsgQueue::msg_notify_obj);
        return true;
    }

    Message Pop() {
        struct ScopeLock {
            ScopeLock() {
                MsgQueue::get_lock(MsgQueue::lock_obj);
            }
            ~ScopeLock() {
                MsgQueue::release_lock(MsgQueue::lock_obj);
            }
            void Unlock() {
                MsgQueue::release_lock(MsgQueue::lock_obj);
            }
            void Lock() {
                MsgQueue::get_lock(MsgQueue::lock_obj);
            }
        } s;

        if (count_ == 0) {
            s.Unlock();
            MsgQueue::wait(MsgQueue::msg_notify_obj);
            s.Lock();
        }

        auto rrpos = read_pos_;
        read_pos_ = (read_pos_ + 1) & (kMaxMessageCount - 1);
        --count_;
        return messages_[rrpos];
    }
private:
    MsgQueue() = default;

    static constexpr auto kMaxMessageCount = 256;
    std::array<Message, kMaxMessageCount> messages_{};
    std::size_t read_pos_{};
    std::size_t write_pos_{};
    std::size_t count_{};
};
