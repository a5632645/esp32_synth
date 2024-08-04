#include "component.h"

void Component::AddChild(Component* child) {
    if (child == nullptr || child->parent_ == this || child == this)
        return;

    if (child->parent_ != nullptr)
        child->parent_->RemoveChild(child);

    child->parent_ = this;
    child->flags_.focus_ = FocusState::kFoucusParent;
    children_.push_back(child);
}

void Component::RemoveChild(Component* child) {
    if (child == nullptr || child->parent_ != this || child == this)
        return;

    child->parent_ = nullptr;
    child->flags_.focus_ = FocusState::kFoucusSelf;
    children_.erase(std::remove(children_.begin(), children_.end(), child));
}

void Component::RemoveChild(size_t index) {
    if (index >= children_.size())
        return;

    auto* child = children_[index];
    child->parent_ = nullptr;
    child->flags_.focus_ = FocusState::kFoucusSelf;
    children_.erase(children_.begin() + index);
}

void Component::RemoveAllChild() {
    for (auto* child : children_) {
        child->parent_ = nullptr;
    }
    children_.clear();
}

void Component::ReplaceChild(size_t index, Component* child) {
    if (index >= children_.size() || child->parent_ == this || child == this)
        return;

    if (child->parent_ != nullptr)
        child->parent_->RemoveChild(child);

    auto* old = children_[index];
    auto old_bound = old->GetBoundInParent();
    old->parent_ = nullptr;
    old->flags_.focus_ = FocusState::kFoucusSelf;
    children_[index] = child;
    child->parent_ = this;
    child->flags_.focus_ = FocusState::kFoucusParent;
    child->SetBound(old_bound);
}

void Component::SetBound(Bound bound) {
    if (bound_parent_ == bound) {
        InternalRepaint(global_bound_);
        return;
    }

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
        child->flags_.focus_ = FocusState::kFoucusSelf;
        child->SetBound(child->GetLocalBound());
    }
    flags_.focus_ = FocusState::kFoucusSelf;
}

void Component::BringToFront() {
    if (parent_ == nullptr)
        return;

    auto it = std::find(parent_->children_.begin(), parent_->children_.end(), this);
    if (it == parent_->children_.end())
        return;

    std::swap(*it, parent_->children_.back());
    Repaint();
}

void Component::BringToBack() {
    if (parent_ == nullptr)
        return;

    auto it = std::find(parent_->children_.begin(), parent_->children_.end(), this);
    if (it == parent_->children_.end())
        return;

    std::swap(*it, parent_->children_.front());
    Repaint();
}

void Component::InternalPaint(Graphic &g, Bound repaint_bound)
{
    auto intersection = global_bound_.GetIntersectionUncheck(repaint_bound);
    if(!intersection.IsValid())
        return;

    g.SetClipBoundGlobal(intersection);
    g.SetComponentBound(global_bound_);
    DrawSelf(g);
    for (auto* child : children_)
        child->InternalPaint(g, intersection);
    
    g.SetClipBoundGlobal(intersection);
    g.SetComponentBound(global_bound_);
    DrawAboveChild(g);
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