#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <vector>
#include <algorithm>
#include <string.h>
#include "graphic.h"
#include "msg_queue.h"
#include "ll_context.h"

class Component {
public:
    Component(LLContext* context = nullptr) : context_(context) {}

    virtual void PaintSelf(Graphic& g) = 0;
    virtual void Resized() = 0;
    
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
    Bound GetBoundTopParent() { return bound_top_parent_; }

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
        bound_parent_ = bound;
        if(parent_ == nullptr) {
            bound_top_parent_ = bound;
        }
        else {
            auto parent_bound = parent_->GetBoundTopParent();
            parent_bound.Shifted(bound.x_, bound.y_);
            parent_bound.w_ = bound.w_;
            parent_bound.h_ = bound.h_;
            bound_top_parent_ = parent_bound;
        }
        Resized();
    }

    void PaintAll(Graphic& g) {
        g.SetClipBound(bound_top_parent_);
        PaintSelf(g);
        for (auto* child : children_)
            child->PaintAll(g);
    }

    virtual ~Component() {
        for (auto* child : children_) {
            child->parent_ = nullptr;
            child->SetBound(child->GetLocalBound());
        }
    }

    void Repaint(Bound bound) {
        auto b = bound_top_parent_.Shift(bound.x_, bound.y_);
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
private:
    void InternalRepaint(Bound repaint_bound) {
        if (parent_ == nullptr) {
            if(context_ != nullptr) {
                MsgQueue::Message msg{
                    .command = cmds::kPaint,
                    .handler = [this, repaint_bound, bb = bound_parent_, c = context_](void* data) {
                        Graphic g{bb, *c};
                        this->InternalPaint(g, repaint_bound);
                        bool flush_screen = true;
                        memcpy(data, &flush_screen, sizeof(flush_screen));
                        if (flush_screen)
                            c->FlushScreen(repaint_bound.x_, repaint_bound.y_, repaint_bound.w_, repaint_bound.h_);
                    }
                };
                bool flush_screen = true;
                memcpy(msg.data.data(), &flush_screen, sizeof(flush_screen));
                MsgQueue::GetInstance().Push(std::move(msg));
            }
        }
        else {
            parent_->InternalRepaint(repaint_bound);
        }
    }

    void InternalPaint(Graphic& g, Bound repaint_bound) {
        auto intersection = bound_top_parent_.GetIntersection(repaint_bound);
        if(intersection.w_ == 0 || intersection.h_ == 0)
            return;

        g.SetClipBound(intersection);
        g.SetComponentBound(bound_top_parent_);
        PaintSelf(g);
        for (auto* child : children_)
            child->InternalPaint(g, repaint_bound);
    }

    std::vector<Component*> children_;
    LLContext* context_ = nullptr;
    Component* parent_ = nullptr;
    Bound bound_parent_;     // bound in parent
    Bound bound_top_parent_; // bound in top level parent
};