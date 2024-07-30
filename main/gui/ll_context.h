#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include "color.h"
#include "bound.h"

#ifndef __cplusplus
typedef struct {
    void* context_ = nullptr;
    void(*SetColor)(void* context, int x, int y, MyColor c) = nullptr;
    void(*FillColorHorizenLine)(void* context, int y, int x, int w, MyColor c) = nullptr;
    void(*FillColorVeticalLine)(void* context, int x, int y, int h, MyColor c) = nullptr;
    void(*FillColorRect)(void* context, int x, int y, int w, int h, MyColor c) = nullptr;
    void(*FlushScreen)(void* context, int x, int y, int w, int h) = nullptr;
    Bound(*GetBound)(void* context) = nullptr;
} LLContext; // why not use inheritance?
#else

/**
 * @brief low level screen driver context
 * 
 * you have to implement these method for your screen driver
 * all the coordinate is limite in the bound you provide
 */
class LLContext {
public:
    virtual ~LLContext() = default;

    virtual void SetColor(int x, int y, MyColor c) = 0;
    virtual void FillColorHorizenLine(int y, int x, int w, MyColor c) = 0;
    virtual void FillColorVeticalLine(int x, int y, int h, MyColor c) = 0;
    virtual void FillColorRect(int x, int y, int w, int h, MyColor c) = 0;
    virtual void FillColorHorizenLineMask(int y, int x, int w, uint8_t* mask, MyColor c) = 0;
    virtual void FlushScreen(int x, int y, int w, int h) = 0;
    virtual Bound GetBound() const = 0;
    // TODO: add mask or batch set for graphic font drawing
};
#endif