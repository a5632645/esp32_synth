#include "component_peer.h"
#include "component.h"

#define OPTION_PEER_SIMPLE_MERGE 0

void ComponentPeer::AddInvalidRect(Bound bound) {
    if (!bound.IsValid())
        return;
        
    if (!bybass_cache) {
        for (auto& b : invalid_rects_) { // merge invalid_rects_
            if (b.Contain(bound))
                return;

            if (bound.Contain(b)) {
                b = bound;
                return;
            }

            auto intersection = b.GetIntersectionUncheck(bound);
            if (!intersection.IsValid())
                continue;

#if OPTION_PEER_SIMPLE_MERGE == 1
            // add outside subtract bound
            Bound outside;
            if (intersection.w_ == bound.w_) {
                outside.x_ = bound.x_;
                outside.w_ = bound.w_;
                outside.h_ = bound.h_ - intersection.h_;
                if (intersection.y_ == bound.y_) {
                    outside.y_ = bound.y_ + intersection.h_;
                }
                else {
                    outside.y_ = bound.y_;
                }
                invalid_rects_.push_back(outside);
                return;
            }
            else if (intersection.h_ == bound.h_) {
                outside.y_ = bound.y_;
                outside.h_ = bound.h_;
                outside.w_ = bound.w_ - intersection.w_;
                if (intersection.x_ == bound.x_) {
                    outside.x_ = bound.x_ + intersection.w_;
                }
                else {
                    outside.x_ = bound.x_;
                }
                invalid_rects_.push_back(outside);
                return;
            }
            else {
                continue;
            }
#else
            Bound larger;
            larger.x_ = std::min(bound.x_, b.x_);
            larger.y_ = std::min(bound.y_, b.y_);
            larger.w_ = std::max(bound.x_ + bound.w_, b.x_ + b.w_) - larger.x_;
            larger.h_ = std::max(bound.y_ + bound.h_, b.y_ + b.h_) - larger.y_;
            b = larger;
            return;
#endif
        }
        invalid_rects_.push_back(bound);
    }
    else {
        Graphic g{*context_};
        owner_->InternalPaint(g, bound);
        context_->FlushScreen(bound.x_, bound.y_, bound.w_, bound.h_);
    }
}

void ComponentPeer::FlushInvalidRects() {
    if (invalid_rects_.empty())
        return;
    if (owner_ == nullptr)
        return;
    if (context_ == nullptr)
        return;

    Graphic g{*context_};
    for (auto& b : invalid_rects_) {
        owner_->InternalPaint(g, b);
        context_->FlushScreen(b.x_, b.y_, b.w_, b.h_);
    }
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
