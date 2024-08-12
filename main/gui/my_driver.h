#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include "bound.h"
#include "my_frame.h"

class MyDriver {
public:
    virtual ~MyDriver() = default;

    virtual void BeginFrame() = 0;
    virtual void AeraDrawed(const Bound& bound) = 0;
    virtual void EndFrame(const Bound& bound) = 0;

    virtual MyFrame& GetFrame() = 0;
};