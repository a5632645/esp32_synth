#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <vector>
#include <algorithm>
#include "graphic.h"
#include "component_peer.h"

class Component {
public:
    virtual void PaintSelf(Graphic& g) {}
    virtual void Resized() {}
    
    /**
     * @brief get local bound
     * @return local bound
     */
    Bound GetLocalBound() {
         return Bound{0, 0, bound_parent_.w_, bound_parent_.h_};
    }

    /**
     * @brief get bound in top level parent
     * @return bound
     */
    Bound GetBoundTopParent() { return global_bound_; }

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

    void SetBound(int x, int y, int w, int h) {
        SetBound(Bound{x, y, w, h});
    }

    void SetBound(Bound bound) {
        if (bound_parent_ == bound)
            return;

        InternalRepaint(global_bound_); // invalid old aera
        
        bound_parent_ = bound;
        if(parent_ == nullptr) {
            global_bound_ = bound;
        }
        else {
            auto parent_bound = parent_->GetBoundTopParent();
            parent_bound.Shifted(bound.x_, bound.y_);
            parent_bound.w_ = bound.w_;
            parent_bound.h_ = bound.h_;
            global_bound_ = parent_bound;
        }
        InternalRepaint(global_bound_); // repaint new aera
        Resized();
    }

    virtual ~Component() {
        for (auto* child : children_) {
            child->parent_ = nullptr;
            child->SetBound(child->GetLocalBound());
        }
    }

    void Repaint(Bound bound) {
        auto b = global_bound_.Shift(bound.x_, bound.y_);
        b.w_ = bound.w_;
        b.h_ = bound.h_;

        if (parent_ != nullptr)
            parent_->InternalRepaint(b);
        else
            InternalRepaint(b);
    }

    void Repaint() {
        Repaint(GetLocalBound());
    }

    Component* GetParent() const { return parent_; }
private:
    friend class ComponentPeer;

    void InternalPaint(Graphic& g, Bound repaint_bound) {
        auto intersection = global_bound_.GetIntersectionUncheck(repaint_bound);
        if(!intersection.IsValid())
            return;

        g.SetClipBoundGlobal(intersection);
        g.SetComponentBound(global_bound_);
        PaintSelf(g);
        for (auto* child : children_)
            child->InternalPaint(g, intersection);
    }

    void InternalRepaint(Bound repaint_bound) {
        if (parent_ != nullptr) {
            parent_->InternalRepaint(repaint_bound);
            return;
        }

        if (peer_ == nullptr)
            return;

        peer_->AddInvalidRect(repaint_bound);
    }

    std::vector<Component*> children_;
    ComponentPeer* peer_ = nullptr;
    Component* parent_ = nullptr;
    Bound bound_parent_;     // bound in parent
    Bound global_bound_; // bound in top level parent
};