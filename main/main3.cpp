#include <stdio.h>
#include <cmath>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <numbers>
#include <string>
#include <esp_timer.h>
#include "model/cordic.h"

#include "gui/msg_queue.h"
#include "gui/component_peer.h"
#include "gui/component.h"
#include "st7735_ll_contex.h"

static void TwoTask2(void*) {
    CoridcData test {};
    CoridcParamStruct reset {};
    CoridcFreqStruct freq {};

    reset.freq[0] = MyFpS0_15_FromFloat(0.1f);
    Coridc_Reset(&test, 1, &reset);

    // ll driver init
    static St7735LLContext ll_context;
    spi_master_init(&ll_context.dev, 2, 1, 41, 40, 42);
    lcdInit(&ll_context.dev, 128, 160, 0, 0);
    lcdDisplayOn(&ll_context.dev);

    // gui init
    Graphic g{ll_context};
    g.SetClipBoundGlobal(ll_context.GetBound());
    g.SetComponentBound(ll_context.GetBound());

    int x = 0;
    int scale = ll_context.kHeight / 2;

    MyPoint p {};
    MyPoint p2 {};
    float freqf = 0.1f;
    auto tick_begin = esp_timer_get_time();
    for (;;) 
    {
        // freq.freq[0] = (freq.freq[0] + 1) & 32767;
        // Coridc_SetFreq(&test, 1, &freq);
        int32_t tmp = 0;
        Coridc_Tick(&test, 1, &tmp);

        auto tick_consumed = esp_timer_get_time() - tick_begin;

        auto t = MyFpS1_14_ToFloat(test.y0[0]);
        auto y = t * scale + scale;

        MyPoint pp {x, (int)y};
        g.SetColor(colors::kWhite);
        g.DrawLine(p, pp);
        p = pp;

        g.SetColor(colors::kRed);
        t = (float)tmp / (1 << 25);
        y = t * scale + scale;
        MyPoint pp2 {x, (int)y};
        g.DrawLine(p2, pp2);
        p2 = pp2;

        // g.SetPixel({x, (int)y});
        ll_context.EndFrame(ll_context.GetBound());

        ++x;
        if (x >= ll_context.kWidth) {
            x = 0;
            g.Fill(colors::kBlack);
            g.DrawSingleLineText(std::to_string(tick_consumed) + "us", 0, 0);

            freqf += 0.01f;
            if (freqf > 0.9f)
                freqf = 0.1f;
            freq.freq[0] = MyFpS0_15_FromFloat(freqf);
            Coridc_SetFreq(&test, 1, &freq);
            g.DrawSingleLineText(std::to_string(freqf), 0, 8);
        }

        vTaskDelay(1);
        tick_begin = esp_timer_get_time();
    }
    vTaskDelete(nullptr);
}

extern "C" void app_main() {
    // xTaskCreate(TwoTask, "TwoTask", 4096, nullptr, 5, nullptr);
    xTaskCreate(TwoTask2, "TwoTask", 4096, nullptr, 5, nullptr);
}