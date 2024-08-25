#include "my_frame.h"
#include "my_render.cpp.impl"
#include "my_color_trait.h"

template class MyRender<MyColorTraits<MyRGB565>>;
template class MyRender<MyColorTraits<MyRGB888>>;
template class MyRender<MyColorTraits<MyBGR565>>;

std::unique_ptr<IMyRender> CreateMyRender(MyFrame& frame) {
    switch (frame.color_type_) {
    case MyColorTraits<MyRGB565>::kType:
        return std::make_unique<MyRender<MyColorTraits<MyRGB565>>>(frame);
    case MyColorTraits<MyRGB888>::kType:
        return std::make_unique<MyRender<MyColorTraits<MyRGB888>>>(frame);
    case MyColorTraits<MyBGR565>::kType:
        return std::make_unique<MyRender<MyColorTraits<MyBGR565>>>(frame);
    default:
        return CreateMyRenderExtra(frame);
    }
}