#include "component.h"

void Component::AddChild(Component* child) {
    if (child == nullptr || child->parent_ == this || child == this)
        return;

    if (child->parent_ != nullptr)
        child->parent_->RemoveChild(child);

    child->parent_ = this;
    child->SetBound(child->GetLocalBound());
    children_.push_back(child);
}

void Component::RemoveChild(Component* child) {
    if (child == nullptr || child->parent_ != this || child == this)
        return;

    child->parent_ = nullptr;
    child->SetBound(child->GetLocalBound());
    children_.erase(std::remove(children_.begin(), children_.end(), child), children_.end());
}

void Component::SetBound(Bound bound) {
    if (bound_parent_ == bound)
        return;

    InternalRepaint(global_bound_); // invalid old aera
    
    bound_parent_ = bound;
    if(parent_ == nullptr) {
        global_bound_ = bound;
    }
    else {
        global_bound_ = parent_->GetBoundTopParent();
        global_bound_.Shifted(bound.x_, bound.y_);
        global_bound_.w_ = bound.w_;
        global_bound_.h_ = bound.h_;
    }
    InternalRepaint(global_bound_); // repaint new aera
    Resized();
}

Component::~Component() {
    for (auto* child : children_) {
        child->parent_ = nullptr;
        child->SetBound(child->GetLocalBound());
    }
}

void Component::InternalPaint(Graphic& g, Bound repaint_bound) {
    auto intersection = global_bound_.GetIntersectionUncheck(repaint_bound);
    if(!intersection.IsValid())
        return;

    g.SetClipBoundGlobal(intersection);
    g.SetComponentBound(global_bound_);
    PaintSelf(g);
    for (auto* child : children_)
        child->InternalPaint(g, intersection);
}

void Component::InternalRepaint(Bound repaint_bound) {
    if (parent_ != nullptr) {
        parent_->InternalRepaint(repaint_bound);
        return;
    }

    if (peer_ == nullptr)
        return;

    peer_->AddInvalidRect(repaint_bound);
}