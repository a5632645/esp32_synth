#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <vector>
#include <cstdint>
#include <functional>
#include <array>
#include "gui_config.h"

class MsgQueue {
public:
    struct Message {
        int command = 0;
        std::function<void()> handler;
    };

    /**
     * @brief  get a global message queue
     * @return the global message queue
     * @note   the message queue is not singleton, you can create multiple, but prefer to use this for gui
     */
    static MsgQueue& GetInstance() {
        static MsgQueue instance;
        return instance;
    }

    
    /**
     * @brief constructor
     */
    MsgQueue() {
        messages_.reserve(kMaxMessageCount);
        batch_messages_.reserve(kMaxMessageCount);
    }

    /**
     * @brief push a message, it's thread safe
     * @param message the message
     * @return true if success, false if queue is full 
     */
    bool Push(Message message) {
        MyScopeLock s { msg_lock_obj_ };
        return PushUnlock(std::move(message));
    }

    /**
     * @brief push a message, it's not thread safe
     * @param message the message
     * @return true if success, false if queue is full
     */
    bool PushUnlock(Message message) {
        if (messages_.size() >= kMaxMessageCount || !message.handler)
            return false;

        messages_.emplace_back(std::move(message));
        msg_notify_obj_.Notify();
        return true;
    }

    /**
     * @brief get last message, it's not thread safe
     * @return the last message, nullptr if queue is empty
     */
    Message* GetLastMsgUnlock() {
        if (messages_.empty())
            return nullptr;
        return &messages_.back();
    }

    /**
     * @brief  swap messages_ and batch_messages_
     * @return is there any message to dispatch
     */
    bool CollectMessageIf() {
        MyScopeLock s { msg_lock_obj_ };

        batch_messages_.swap(messages_);
        return !batch_messages_.empty();
    }

    /**
     * @brief block until there is a message
     */
    void WaitMessage() {
        MyScopeLock s { msg_lock_obj_ };

        if (messages_.empty()) {
            s.Unlock();
            msg_notify_obj_.Wait();
        }
    }

    /**
     * @brief dispatch all messages, is not thread safe
     */
    void DispatchMessage() {
        for (auto& msg : batch_messages_) {
            msg.handler();
        }
        batch_messages_.clear();
    }
private:
    static constexpr auto kMaxMessageCount = 64;
    std::vector<Message> messages_{};
    std::vector<Message> batch_messages_{};

    // lock
    MyLock msg_lock_obj_;
    MyNotify msg_notify_obj_;
};
