#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <vector>
#include <cstdint>
#include <functional>
#include <array>

class MsgQueue {
private:
    inline static bool has_message = false;
    inline static bool lock = false;

    static void DummyWait(void*) {
        while (!has_message) {}
        has_message = false;
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

    struct MsgQueueLock {
        MsgQueueLock() {
            MsgQueue::get_lock(MsgQueue::lock_obj);
        }
        ~MsgQueueLock() {
            MsgQueue::release_lock(MsgQueue::lock_obj);
        }
        void Unlock() {
            MsgQueue::release_lock(MsgQueue::lock_obj);
        }
        void Lock() {
            MsgQueue::get_lock(MsgQueue::lock_obj);
        }
    };

    struct Message {
        static constexpr auto kMaxMessageDataByte = 32;

        int command = 0;
        std::array<uint8_t, kMaxMessageDataByte> data {};
        std::function<void(void*)> handler;
    };

    static MsgQueue& GetInstance() {
        static MsgQueue instance;
        return instance;
    }

    bool Push(Message message) {
        MsgQueueLock s;
        return PushUnlock(std::move(message));
    }

    bool PushUnlock(Message message) {
        if (count_ == kMaxMessageCount)
            return false;

        messages_.emplace_back(std::move(message));
        ++count_;
        MsgQueue::notify(MsgQueue::msg_notify_obj);
        return true;
    }

    Message* GetLastMsgUnlock() {
        if (count_ == 0)
            return nullptr;
        return messages_.data() + count_ - 1;
    }

    void Loop() {
        for (;;) {
            while (!CollectMessageIf()) {
                WaitMessage();
            }
            DispatchMessage();
        }
    }

    bool CollectMessageIf() {
        MsgQueueLock s;

        batch_messages_.swap(messages_);
        count_ = 0;
        return !batch_messages_.empty();
    }

    void WaitMessage() {
        MsgQueueLock s;

        if (count_ == 0) {
            s.Unlock();
            MsgQueue::wait(MsgQueue::msg_notify_obj);
        }
    }

    void DispatchMessage() {
        for (auto& msg : batch_messages_) {
            msg.handler(msg.data.data());
        }
        batch_messages_.clear();
    }
private:
    MsgQueue() {
        messages_.reserve(kMaxMessageCount);
        batch_messages_.reserve(kMaxMessageCount);
    }

    static constexpr auto kMaxMessageCount = 64;
    std::vector<Message> messages_{};
    std::vector<Message> batch_messages_{};
    std::size_t count_{};
};
