#include "component_peer.h"
#include "component.h"

void ComponentPeer::AddInvalidRect(Bound bound) {
    bound = bound.GetIntersectionUncheck(context_->GetBound());

    if (!bound.IsValid())
        return;
        
    for (auto& b : invalid_rects_cache_) { // merge invalid_rects_
        if (b.Contain(bound))
            return;

        if (bound.Contain(b)) {
            b = bound;
            return;
        }

        auto intersection = b.GetIntersectionUncheck(bound);
        if (intersection.w_ < 0 && intersection.h_ < 0) // if the bounds are not intersect at least a pixel
            continue;

        Bound larger;
        larger.x_ = std::min(bound.x_, b.x_);
        larger.y_ = std::min(bound.y_, b.y_);
        larger.w_ = std::max(bound.x_ + bound.w_, b.x_ + b.w_) - larger.x_;
        larger.h_ = std::max(bound.y_ + bound.h_, b.y_ + b.h_) - larger.y_;
        b = larger;
        return;
    }
    invalid_rects_cache_.push_back(bound);
}

void ComponentPeer::FlushInvalidRects() {
    if (invalid_rects_cache_.empty())
        return;
    if (owner_ == nullptr)
        return;
    if (context_ == nullptr)
        return;

    invalid_rects_cache_.swap(invalid_rects_);
    Graphic g{*context_};
    Bound dirty_aera = invalid_rects_.front();
    int left = dirty_aera.x_ + dirty_aera.w_;
    int bottom = dirty_aera.y_ + dirty_aera.h_;
    context_->BeginFrame();
    for (auto& b : invalid_rects_) {
        owner_->InternalPaint(g, b);
        context_->FlushScreen(b);

        dirty_aera.x_ = std::min(dirty_aera.x_, b.x_);
        dirty_aera.y_ = std::min(dirty_aera.y_, b.y_);
        left = std::max(left, b.x_ + b.w_);
        bottom = std::max(bottom, b.y_ + b.h_);
    }
    dirty_aera.w_ = left - dirty_aera.x_;
    dirty_aera.h_ = bottom - dirty_aera.y_;
    context_->EndFrame(dirty_aera);
    invalid_rects_.clear();
}

void ComponentPeer::ChangeComponent(Component *owner) {
    if (owner_ == owner)
        return;

    if (owner_ != nullptr) {
        owner_->peer_ = nullptr;
        owner_ = nullptr;
    }

    owner_ = owner;
    if (owner != nullptr) {
        if (owner->peer_ != nullptr) {
            owner->peer_->owner_ = nullptr;
            owner->peer_->invalid_rects_.clear();
            owner->peer_ = nullptr;
        }
        owner->peer_ = this;
    }

    if (context_ == nullptr)
        return;
    invalid_rects_.clear();
    owner->SetBound(context_->GetBound());
}
