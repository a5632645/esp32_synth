#include "component.h"

void Component::AddChild(Component* child) {
    if (child == nullptr || child->parent_ == this || child == this)
        return;

    if (child->parent_ != nullptr)
        child->parent_->RemoveChild(child);

    child->_SetParent(this);
    children_.push_back(child);
}

void Component::RemoveChild(Component* child) {
    if (child == nullptr || child->parent_ != this || child == this)
        return;

    child->_SetParent(nullptr);
    children_.erase(std::remove(children_.begin(), children_.end(), child));
}

void Component::RemoveChild(size_t index) {
    if (index >= children_.size())
        return;

    auto* child = children_[index];
    child->_SetParent(nullptr);
    children_.erase(children_.begin() + index);
}

void Component::RemoveAllChild() {
    for (auto* child : children_)
        child->_SetParent(nullptr);
    children_.clear();
}

void Component::ReplaceChild(size_t index, Component* child) {
    if (index >= children_.size() || child->parent_ == this || child == this)
        return;

    if (child->parent_ != nullptr)
        child->parent_->RemoveChild(child);

    auto* old = children_[index];
    auto old_bound = old->GetBoundInParent();
    old->_SetParent(nullptr);
    children_[index] = child;
    child->_SetParent(this);
    child->SetBound(old_bound);
}

void Component::SetBound(Bound bound) {
    if (bound_parent_ == bound) {
        _Repaint(global_bound_);
        return;
    }

    _Repaint(global_bound_); // invalid old aera
    
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
    _Repaint(global_bound_); // repaint new aera
    Resized();
}

Component::~Component() {
    RemoveAllChild();
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

void Component::_Paint(MyGraphic& g, Bound repaint_bound) {
    auto intersection = global_bound_.GetIntersectionUncheck(repaint_bound);
    if(!intersection.IsValid())
        return;

    g.SetClipBoundGlobal(intersection);
    g.SetComponentBound(global_bound_);
    DrawSelf(g);
    for (auto* child : children_)
        child->_Paint(g, intersection);

    g.SetClipBoundGlobal(intersection);
    g.SetComponentBound(global_bound_);
    DrawAboveChild(g);
}

void Component::_Repaint(Bound repaint_bound) {
    if (parent_ != nullptr) {
        parent_->_Repaint(repaint_bound);
        return;
    }

    if (peer_ != nullptr)
        peer_->AddInvalidRect(repaint_bound);
}

void Component::_SetParent(Component* parent) {
    if (parent_ == parent)
        return;

    parent_ = parent;
    if (parent_ == nullptr) {
        flags_.focus_ = FocusState::kFoucusSelf;
    }
    else {
        flags_.focus_ = FocusState::kFoucusParent;
    }
}
