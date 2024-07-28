#pragma once

#ifndef __cplusplus
#error only c++ support
#endif

#include <vector>
#include <cstdint>
#include "bound.h"
#include "component.h"

class MainWindow : public Component {
public:
    MainWindow(int w, int h) {
        SetBound(Bound{0, 0, w, h});
    }

    void PaintSelf(Graphic& g) override {
        g.Fill(colors::kBlack);
        g.SetColor(colors::kWhite);
        auto b = GetLocalBound();
        static int sizex = 0;
        static int sizey = 0;
        g.DrawRect(b.WithCenter(sizex, sizey));
        sizey = (sizey + 1) & 255;
        sizex = (sizex + 1) & 255;
    }

    void Resized() override {

    }
};