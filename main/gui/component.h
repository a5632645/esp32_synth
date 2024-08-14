#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <vector>
#include <algorithm>
#include "my_graphic.h"
#include "component_peer.h"

struct MyEvent {
    uint16_t event_type : 16 = 0;
    uint16_t sub_type : 15 = 0;
    bool heap_data : 1 = false;
    union {
        void* ptr_data[4];
        uint8_t plaint_data[sizeof(void*) * 4];
    } data = {0};
};

class Component {
public:
    virtual ~Component();

    // drawing
    virtual void DrawSelf(MyGraphic& g) {}
    virtual void DrawAboveChild(MyGraphic& g) {}
    virtual void Resized() {}

    // event
    virtual void OnEventGet(const MyEvent& e) {}

    /**
     * @brief get local bound
     * @return local bound
     */
    Bound GetLocalBound() const {
         return Bound{0, 0, bound_parent_.w_, bound_parent_.h_};
    }

    Bound GetBoundInParent() const { return bound_parent_; }

    /**
     * @brief get bound in top level parent
     * @return bound
     */
    Bound GetBoundTopParent() const { return global_bound_; }

    void AddChild(Component* child);
    void RemoveChild(Component* child);
    void RemoveChild(size_t index);
    void RemoveAllChild();
    void ReplaceChild(size_t index, Component* child);

    void SetBound(int x, int y, int w, int h) {
        SetBound(Bound{x, y, w, h}); 
    }
    void SetBound(Bound bound);

    void Repaint(Bound bound) {
        auto b = global_bound_.Shift(bound.x_, bound.y_);
        b.w_ = bound.w_;
        b.h_ = bound.h_;
        _Repaint(b);
    }
    void Repaint() {
        Repaint(GetLocalBound()); 
    }

    Component* GetParent() const { return parent_; }
    Component* GetTopParent() { return parent_ == nullptr ? this : parent_->GetTopParent(); }
    ComponentPeer* GetPeer() const { return peer_; }
    size_t GetNumChild() const { return children_.size(); }
    Component* GetChildUncheck(size_t i) const { return children_[i]; }

    void BringToFront();
    void BringToBack();

    virtual void OnGetFocus() {}
    virtual void OnLostFocus() {}
    void GetFocus() {
        if (parent_ != nullptr)
            parent_->ChildGetFocus(this);
    }
    void ExitFocus() {
        if (parent_ != nullptr)
            parent_->ChildExitFocus(this);
    }
    void ChildGetFocus(Component* child) {
        flags_.focus_ = FocusState::kFoucusChild;
        OnLostFocus();
        child->flags_.focus_ = FocusState::kFoucusSelf;
        child->OnGetFocus();
    }
    void ChildExitFocus(Component* child) {
        child->flags_.focus_ = FocusState::kFoucusParent;
        child->OnLostFocus();
        flags_.focus_ = FocusState::kFoucusSelf;
        OnGetFocus();
    }
protected:
    friend class ComponentPeer;

    void _Paint(MyGraphic& g, Bound repaint_bound);
    void _Repaint(Bound repaint_bound);
    void _SetParent(Component* parent);
    
    // bounding
    std::vector<Component*> children_;
    ComponentPeer* peer_ = nullptr;
    Component* parent_ = nullptr;
    Bound bound_parent_; // bound in parent
    Bound global_bound_; // bound in global space(driver space)

    enum class FocusState : uint8_t {
        kFoucusParent = 0,
        kFoucusSelf = 1,
        kFoucusChild = 2
    };

    struct {
        bool visible_ : 1 = true;
        FocusState focus_ : 2 = FocusState::kFoucusSelf;
    } flags_;
};