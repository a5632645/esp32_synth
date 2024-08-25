#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <vector>
#include "bound.h"
#include "my_driver.h"
#include "my_graphic.h"

class Component;

class ComponentPeer {
public:
    ComponentPeer(MyDriver* driver) : driver_(driver) {
        invalid_rects_.reserve(16);
        invalid_rects_cache_.reserve(16);
        buffer_bound_ = driver->GetFrame()->GetBound();
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

    void InvalidScreen() {
        invalid_rects_cache_.clear();
        invalid_rects_cache_.push_back(buffer_bound_);
    }
private:
    std::vector<Bound> invalid_rects_cache_;
    std::vector<Bound> invalid_rects_;
    Component* component_ = nullptr;
    MyDriver* driver_ = nullptr;
    Bound buffer_bound_;
};