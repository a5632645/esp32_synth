#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <vector>
#include "bound.h"
#include "ll_context.h"
#include "graphic.h"

class Component;

class ComponentPeer {
public:
    struct OwnedPtr {
        Component* ptr { nullptr };
        bool delete_layer { false };

        OwnedPtr() = default;
        OwnedPtr(Component* ptr, bool owned) : ptr(ptr), delete_layer(owned) {}

        // copy constructor will cause deleted memory in vector expand
        OwnedPtr(const OwnedPtr&) = delete;
        OwnedPtr& operator=(const OwnedPtr&) = delete;
        OwnedPtr(OwnedPtr&& other) noexcept {
            ptr = other.ptr;
            delete_layer = other.delete_layer;
            other.ptr = nullptr;
            other.delete_layer = false;
        }
        OwnedPtr& operator=(OwnedPtr&& other) noexcept {
            ptr = other.ptr;
            delete_layer = other.delete_layer;
            other.ptr = nullptr;
            other.delete_layer = false;
            return *this;
        }
        ~OwnedPtr();
    };

    ComponentPeer(LLContext* context) : context_(context) {
        invalid_rects_.reserve(16);
        invalid_rects_cache_.reserve(16);
    }

    /**
     * @brief add invalid rect, it will automatically merge with existed invalid rects.
     * @param bound the invalid rect in global space
     * @note it's not thread safe
     */
    void AddInvalidRect(Bound bound);

    /**
     * @brief draw invalid rects and send to the driver
     * @note it's not thread safe
     */
    void FlushInvalidRects();

    /**
     * @brief check if there are invalid rects
     * @return true if there are invalid rects, false if not
     * @note it's not thread safe
     */
    bool HasInvalidRects() const { return !invalid_rects_cache_.empty(); }

    /**
     * @brief set component own the peer, the component will setbound to the driver(if exist)
     * @param owner the component
     * @note it's not thread safe
     */
    void SetComponent(Component* owner);

    /**
     * @brief get component
     * @return the component, nullptr if not exist
     * @note it's not thread safe
     */
    Component* GetComponent() const { return component_; }
private:
    std::vector<Bound> invalid_rects_cache_;
    std::vector<Bound> invalid_rects_;
    Component* component_ = nullptr;
    LLContext* context_ = nullptr;
};