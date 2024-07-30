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
    ComponentPeer(LLContext* context) : context_(context) {}

    void AddInvalidRect(Bound bound);
    void FlushInvalidRects();
    void ChangeComponent(Component* owner);
private:

    std::vector<Bound> invalid_rects_;
    Component* owner_ = nullptr;
    LLContext* context_ = nullptr;
    bool bybass_cache = false;
};