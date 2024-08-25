#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#if 0

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
// if need task/thread sync, need implment these
struct MyLock {
    SemaphoreHandle_t mutex;

    MyLock() {
        mutex = xSemaphoreCreateBinary();
        xSemaphoreGive(mutex);
    }

    void Lock() {
        xSemaphoreTake(mutex, portMAX_DELAY);
    }

    void Unlock() {
        xSemaphoreGive(mutex);
    }
};

struct MyNotify {
    SemaphoreHandle_t sem;

    MyNotify() {
        sem = xSemaphoreCreateBinary();
    }

    void Wait() {
        xSemaphoreTake(sem, portMAX_DELAY);
    }

    void Notify() {
        xSemaphoreGive(sem);
    }
};

struct MyScopeLock {
    MyScopeLock(MyLock& lock) : lock_(lock) {
        lock_.Lock();
    }

    ~MyScopeLock() {
        lock_.Unlock();
    }

    void Lock() {
        lock_.Lock();
    }

    void Unlock() {
        lock_.Unlock();
    }

    MyLock& lock_;
};

#else

struct MyLock {
    bool mutex{false};

    MyLock() = default;

    void Lock() {
        while (mutex) {}
        mutex = true;
    }

    void Unlock() {
        mutex = false;
    }
};

struct MyNotify {
    bool sem{false};

    MyNotify() = default;

    void Wait() {
        while (!sem) {}
    }

    void Notify() {
        sem = true;
    }
};

struct MyScopeLock {
    MyScopeLock(MyLock& lock) : lock_(lock) {
        lock_.Lock();
    }

    ~MyScopeLock() {
        lock_.Unlock();
    }

    void Lock() {
        lock_.Lock();
    }

    void Unlock() {
        lock_.Unlock();
    }

    MyLock& lock_;
};

#endif