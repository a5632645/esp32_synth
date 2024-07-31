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
    virtual ~Component();
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

    void AddChild(Component* child);
    void RemoveChild(Component* child);

    void SetBound(int x, int y, int w, int h) {
        SetBound(Bound{x, y, w, h}); 
    }
    void SetBound(Bound bound);

    void Repaint(Bound bound) {
        auto b = global_bound_.Shift(bound.x_, bound.y_);
        b.w_ = bound.w_;
        b.h_ = bound.h_;
        InternalRepaint(b);
    }
    void Repaint() {
        Repaint(GetLocalBound()); 
    }

    Component* GetParent() const { return parent_; }
    ComponentPeer* GetPeer() const { return peer_; }
    size_t GetNumChild() const { return children_.size(); }
    Component* GetChildUncheck(size_t i) const { return children_[i]; }
private:
    friend class ComponentPeer;

    void InternalPaint(Graphic& g, Bound repaint_bound);

    void InternalRepaint(Bound repaint_bound);

    std::vector<Component*> children_;
    ComponentPeer* peer_ = nullptr;
    Component* parent_ = nullptr;
    Bound bound_parent_; // bound in parent
    Bound global_bound_; // bound in top level parent
};