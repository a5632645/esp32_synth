#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <vector>
#include <algorithm>
#include "graphic.h"

class Component {
public:
    virtual ~Component() = default;
    virtual void PaintSelf(Graphic& g) = 0;
    virtual void Resized() = 0;
    
    Bound GetLocalBound() { return local_bound_; }
    Bound GetBound() { return bound_; }

    void AddChild(Component* child) {
        child->parent_ = this;
        child->SetBound(child->GetLocalBound());
        children_.push_back(child);
    }
    void RemoveChild(Component* child) {
        child->parent_ = nullptr;
        child->SetBound(child->GetLocalBound());
        children_.erase(std::remove(children_.begin(), children_.end(), child), children_.end());
    }
    void SetBound(Bound bound) {
        local_bound_ = bound;
        if(parent_ == nullptr) {
            bound_ = bound;
            return;
        }
        
        auto parent_bound = parent_->GetBound();
        parent_bound.Shifted(bound.x_, bound.y_);
        parent_bound.w_ = bound.w_;
        parent_bound.h_ = bound.h_;
        bound_ = parent_bound;
        Resized();
    }

    void PaintAll(Graphic& g) {
        g.SetTargetBound(bound_);
        PaintSelf(g);
        for (auto* child : children_)
            child->PaintAll(g);
    }
private:
    std::vector<Component*> children_;
    Component* parent_{};
    Bound local_bound_;
    Bound bound_;
};